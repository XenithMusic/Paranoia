#include <stdint.h>
#include "fail.h" // Include the fail.h header
#include "utils.h"
#include "syscall.h"

#define PIC1		0x20		/* IO base address for master PIC */
#define PIC2		0xA0		/* IO base address for slave PIC */
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)

// Simple ISR for handling exceptions
extern "C" {
    void eoi() {
        outb(PIC1_COMMAND, 0x20); // Acknowledge the master PIC
        outb(PIC2_COMMAND, 0x20); // Acknowledge the slave PIC
    }
    void exception_handler() {
        fault(10000,"Generic Exception Handler ISR");
        eoi();
    }
    void divzero_handler() {
        fault(1,"Division by Zero");
        eoi();
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
        eoi();
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