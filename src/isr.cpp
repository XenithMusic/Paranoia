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
        eoi(1);
        if (ps2keyboard::state.state == EnablingScanning) {
            ps2keyboard::state.state = NoProcess;
            ps2keyboard::state.data1 = scancode;
        } else if (ps2keyboard::state.state == WaitingForScancodes) {
            ps2keyboard::state.state = EatingScancode;
            ps2keyboard::state.data1 = scancode;
        } else if (ps2keyboard::state.state == EatingScancode) {
            if (ps2keyboard::state.data2 == 0x00) {
                ps2keyboard::state.data2 = scancode; // Queue up an extra scancode
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