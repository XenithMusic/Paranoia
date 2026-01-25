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
        uint8_t status = inb(0x64);
        bool obf = status & (1 << 0);
        bool ismouse = status & (1 << 5); // HACK: Supposedly, bit 5 is inconsistent on very old hardware during controller transitions.
        if (not obf // Protects against Surpious IRQ1
            or ismouse // Protects against Mouse Leakage
        ) {
            eoi(1);
            return;
        }
        uint8_t scancode = inb(0x60);
        auto &st = ps2keyboard::state;
        st.ready = false;
        if (st.state == WaitingForScancodes or st.state == EatingScancode) {
            st.state = EatingScancode;
            st.lastScancode = (st.lastScancode+1)%64;
            st.data[st.lastScancode] = scancode;
        } else if (st.state == WaitingForAck) {
            if (scancode == 0xFA) {
                st.state = NoProcess;
                st.data[0] = scancode;
            } else if (scancode == 0xFE) {
                st.state = Failure;
            }
        }
        st.ready = true;
        eoi(1);
        return;
    }

    // NOTE: This IRQ handler does too much, and most of the work has
    //       been moved to drivers/ps2 ps2keyboard::processScancodes().
    //
    //       The modern version of this IRQ handler is above, in
    //       irq1_handler().

    // void irq1_handler() {
    //     uint8_t status = inb(0x64);
    //     bool obf = status&(1 << 0);
    //     bool ismouse = status&(1 << 5); // HACK: This may break on very old hardware during controller transitions.
    //     if (not obf // Protects against Surpious IRQ1
    //         or ismouse // Protects against Mouse Leakage
    //     ) {
    //         eoi(1);
    //         return;
    //     }
    //     uint8_t scancode = inb(0x60);
    //     uint64_t proc;
    //     auto &st = ps2keyboard::state;
    //     PS2_STATES newState = st.state;
    //     if (newState == Failure or newState == Unimplemented) {
    //     } else if (newState == WaitingForAck) {
    //         // Terminal::print("Whoa!\n");
    //         // if (scancode == 0xFA) {
    //         //     Terminal::print("ACK\n");
    //         // }
    //         if (scancode == 0xFE) {
    //             st.state = Unimplemented;
    //             eoi(1);
    //             return;
    //         }
    //         st.data[0] = scancode;
    //         st.lastScancode = 1; // discard all data because i sorta have to
    //         st.state = NoProcess;
    //     } else if (newState == WaitingForScancodes or newState == EatingScancode) {
    //         if (st.lastScancode > 4) { // check if there is space in the 'queue'
    //             st.overwhelmed = true; // i cannot do anything further here; this is a fail condition
    //             eoi(1);
    //             return;
    //         }
    //         proc = st.data[st.lastScancode];
    //         if (proc == 0) {
    //             st.remainingScancodes = 0;
    //             st.flags = 0;
    //         }
    //         proc <<= 8;
    //         proc |= scancode;
    //         if (st.remainingScancodes > 0) st.remainingScancodes--;
    //         if ((st.flags&SCANCODE_GUARANTEE) == 0) { // scancode is not guaranteed
    //             if (st.scancodeSet == 1) {
    //                 if (proc == 0xE02A) { // guaranteed to be E0 2A E0 37 - prtsc press
    //                     st.flags |= SCANCODE_GUARANTEE; // bit 3: guaranteed
    //                     st.remainingScancodes = 2;
    //                 } else if (proc == 0xE0B7) { // guaranteed to be E0 B7 E0 AA - prtsc release
    //                     st.flags |= SCANCODE_GUARANTEE; // bit 3: guaranteed
    //                     st.remainingScancodes = 2;
    //                 } else if (proc == 0xE1) { // guaranteed to be E1 1D 45 E1 9D C5 - pause press, pause release
    //                     st.flags |= SCANCODE_GUARANTEE; // bit 3: guaranteed
    //                     st.remainingScancodes = 5;
    //                 }
    //             } else if (st.scancodeSet == 2) {
    //                 if (proc == 0xE012) { // guaranteed to be E0 12 E0 7C - prtsc press
    //                     st.flags |= SCANCODE_GUARANTEE; // bit 3: guaranteed
    //                     st.remainingScancodes = 2;
    //                 } else if (proc == 0xE0F07C) { // guaranteed to be E0 F0 7C E0 F0 12 - prtsc release
    //                     st.flags |= SCANCODE_GUARANTEE; // bit 3: guaranteed
    //                     st.remainingScancodes = 3;
    //                 } else if (proc == 0xE1) { // guaranteed to be E1 14 77 E1 F0 14 F0 77 - pause press, pause release
    //                     st.flags |= SCANCODE_GUARANTEE; // bit 3: guaranteed
    //                     st.remainingScancodes = 7;
    //                 }
    //             }
    //             if (scancode == 0xE0) { // EXTENDED
    //                 st.flags |= SCANCODE_EXTENDED; // bit 1: extended
    //                 st.remainingScancodes = 1;
    //             } else if (scancode == 0xF0) { // RELEASES
    //                 st.flags |= SCANCODE_RELEASED; // bit 1: released
    //                 st.remainingScancodes = 1;
    //             }
    //         }
    //         st.data[st.lastScancode] = proc;
    //         if (st.remainingScancodes == 0) {
    //             st.lastScancode++;
    //             st.state = EatingScancode;
    //         }
    //     }
    //     eoi(1);
    // }
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