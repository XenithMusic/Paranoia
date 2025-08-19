#include <stdint.h>
#include "fail.h" // Include the fail.h header
#include "utils.h"
#include "types.h"
#include "syscall.h"
#include "drivers/ps2.h"

#define PIC1		0x20		/* IO base address for master PIC */
#define PIC2		0xA0		/* IO base address for slave PIC */
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)

#define SCANCODE_EXTENDED 0b00000001
#define SCANCODE_RELEASED 0b00000010
#define SCANCODE_GUARANTEE 0b00000100

// Simple ISR for handling exceptions
extern "C" {
    void beginInterrupt() {

    }
    void endInterrupt() {

    }
    volatile void eoi(uint8_t irq) {
        if (irq >= 8) {
            outb(PIC2_COMMAND,0x20);
        }
        outb(PIC1_COMMAND, 0x20);
    }
    void exception_handler(uint32_t int_no, uint32_t err_code) {
        fault(int_no,"Generic Exception Handler ISR");
        // eoi(); DO NOT CALL EOI HERE; CPU EXCEPT
    }
    void divzero_handler() {
        fault(1,"Division by Zero");
        // eoi(); DO NOT CALL EOI HERE; CPU EXCEPT
    }
    void genprotfault_handler() {
        fault(1, "General Protection Fault");
    }
    void basic_eoi_assembly_low();
    void basic_eoi_assembly_high();
    void irq1_assembly();
    void irq1_handler() {
        volatile uint8_t scancode = inb(0x60);
        uint64_t proc;
        auto &st = ps2keyboard::state;
        PS2_STATES newState = st.state;
        eoi(1);
        if (newState == WaitingForAck) {
            // Terminal::print("Whoa!\n");
            // if (scancode == 0xFA) {
            //     Terminal::print("ACK\n");
            // }
            st.data[0] = scancode;
            st.queueSize = 1; // discard all data because i sorta have to
            st.state = NoProcess;
        } else if (newState == WaitingForScancodes or newState == EatingScancode) {
            proc = st.data[st.queueSize];
            if (proc == 0) {
                st.stateInfo1 = 0;
                st.stateInfo2 = 0;
            }
            proc <<= 8;
            proc |= scancode;
            if (st.stateInfo1 > 0) st.stateInfo1--;
            if (st.queueSize > 4) { // check if there is space in the 'queue'
                st.overwhelmed = true; // i cannot do anything further here; this is a fail condition
                return;
            }
            if ((st.stateInfo2&SCANCODE_GUARANTEE) == 0) { // scancode is not guaranteed
                if (st.scancodeSet == 1) {
                    if (proc == 0xE02A) { // guaranteed to be E0 2A E0 37 - prtsc press
                        st.stateInfo2 |= SCANCODE_GUARANTEE; // bit 3: guaranteed
                        st.stateInfo1 = 2;
                    } else if (proc == 0xE0B7) { // guaranteed to be E0 B7 E0 AA - prtsc release
                        st.stateInfo2 |= SCANCODE_GUARANTEE; // bit 3: guaranteed
                        st.stateInfo1 = 2;
                    } else if (proc == 0xE1) { // guaranteed to be E1 1D 45 E1 9D C5 - pause press, pause release
                        st.stateInfo2 |= SCANCODE_GUARANTEE; // bit 3: guaranteed
                        st.stateInfo1 = 5;
                    }
                } else if (st.scancodeSet == 2) {
                    if (proc == 0xE012) { // guaranteed to be E0 12 E0 7C - prtsc press
                        st.stateInfo2 |= SCANCODE_GUARANTEE; // bit 3: guaranteed
                        st.stateInfo1 = 2;
                    } else if (proc == 0xE0F07C) { // guaranteed to be E0 F0 7C E0 F0 12 - prtsc release
                        st.stateInfo2 |= SCANCODE_GUARANTEE; // bit 3: guaranteed
                        st.stateInfo1 = 3;
                    } else if (proc == 0xE1) { // guaranteed to be E1 14 77 E1 F0 14 F0 77 - pause press, pause release
                        st.stateInfo2 |= SCANCODE_GUARANTEE; // bit 3: guaranteed
                        st.stateInfo1 = 7;
                    }
                }
                if (scancode == 0xE0) { // EXTENDED
                    st.stateInfo2 |= SCANCODE_EXTENDED; // bit 1: extended
                    st.stateInfo1 = 1;
                } else if (scancode == 0xF0) { // RELEASES
                    st.stateInfo2 |= SCANCODE_RELEASED; // bit 1: released
                    st.stateInfo1 = 1;
                }
            }
            st.data[st.queueSize] = proc;
            if (st.stateInfo1 == 0) {
                st.queueSize++;
                st.state = EatingScancode;
            }
        }
    }
    void basic_eoi_handler_low() {
        eoi(1);
    }
    void basic_eoi_handler_high() {
        eoi(9);
    }
    void syscall() {
        int syscall_num;
        int arg1, arg2, arg3;

        // Use registers to retrieve syscall arguments
        asm volatile (
            "movl %%eax, %0\n\t"  // Move syscall_num from EAX
            "movl %%ebx, %1\n\t"  // Move arg1 from EBX
            "movl %%ecx, %2\n\t"  // Move arg2 from ECX
            "movl %%edx, %3\n\t"  // Move arg3 from EDX
            : "=m"(syscall_num), "=m"(arg1), "=m"(arg2), "=m"(arg3) // Output operands
            : // No input operands
            : "eax", "ebx", "ecx", "edx" // Clobbered registers
        );

        // Handle system calls here
        handleSyscall((enum syscall_id)syscall_num, arg1, arg2, arg3);
        // For now, just acknowledge the interrupt
        // eoi(); DO NOT CALL EOI HERE; SOFTWARE INTERRUPT
        return;
    }
	void syscall_c(int id,int arg1,int arg2,int arg3) {
		__asm__ __volatile__ (
			"movl %0, %%eax\n\t"
			"movl %1, %%ebx\n\t"
			"movl %2, %%ecx\n\t"
			"movl %3, %%edx\n\t"
			:
			: "m" (id), "m" (arg1), "m" (arg2), "m" (arg3)
			: "%eax", "%ebx", "%ecx", "%edx"
		);
		syscall();
	}
}