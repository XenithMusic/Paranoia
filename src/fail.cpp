#include "terminal.h"
#include "string.h"
#include "pit.h"
#include "utils.h"

/*

Copyright (C) 2026  XenithMusic (on github)

The Paranoia kernel is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Paranoia is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Paranoia. If not, see <https://www.gnu.org/licenses/>.

*/


int error;
char* str_fail; // pointer if anything demands a pointer

void stackTrace(unsigned int MaxFrames)
{

    struct stackframe *stk;
    asm ("movl %%ebp,%0" : "=r"(stk) ::);
    for(unsigned int frame = 0; stk && frame < MaxFrames; ++frame)
    {
        // Unwind to previous stack frame
        Terminal::print("    0x");
		Terminal::print(parseU32(stk->eip,str_fail,16));
        Terminal::print("\n");
        stk = stk->ebp;
    }
}

void setError(int errno) {
	if (errno == 1000) {
		induceHalt();
	} else {
		error = errno;
	}
}

void fault(int errno,char* string);

void assert(bool dontcrash) {
	if (dontcrash == false) {
		fault(1,"An assertion was triggered.");
	}
}

int getError() {
	return error;
}

void clearError() {
	error = 0;
}

void fault(int errno,char* string) {
	cli(); // prevent interrupts causing unexpected fault exiting
	char* str;
	Terminal::init();
	Terminal::clearScreen();
	Terminal::setRow(0);
	Terminal::print("A significant error (a FAULT) occurred in the Paranoia\n");
		Terminal::print("kernel, and it needed to stop executing immediately.\n\n");
	Terminal::print("FAULT INFO\n");
	Terminal::print("  Code:  ");
		Terminal::print(parseInt(errno,str,10));
		Terminal::print("\n");
	if (string != nullptr) {
	Terminal::print("  Extra: ");
		Terminal::print(string);
		Terminal::print("\n");
	}
	Terminal::print("  Stack Trace:\n\n");
	stackTrace(20);
	Terminal::print("\n");
	Terminal::print("Your computer will not restart itself.\n\n");
	Terminal::print("FAULT specific details:\n");
	if (errno == 69420) {
		Terminal::print("  This is an errno intended only for testing\n");
		Terminal::print("  the kernel fault system.\n\n");

		Terminal::print("  If this occurs in a production release, please\n");
		Terminal::print("  inform the developer at <xenith.contact.mail@gmail\n");
		Terminal::print("  .com>, and then downgrade.");
	}
	if (errno == 10000) {
		Terminal::print("  An unspecified error occurred, causing Paranoia to fail.\n\n");
		Terminal::print("  The error that has occurred was not specified, so Paranoia cannot be sure\n");
		Terminal::print("  that it is safe to continue running code.\n");
	}
	if (errno == 1) {
		Terminal::print("  This is a generic errno.\n\n");

		Terminal::print("  Read 'extra' under FAULT INFO for more details.");
	}
	if (errno == -100) {
		Terminal::print("  A segmented memory block did not end.\n\n");

		Terminal::print("  Memory integrity cannot be verified, when a\n");
		Terminal::print("  property of all memory blocks does not exist.");
	}
	if (errno == -101) {
		Terminal::print("  The memory allocator failed to initialize.\n\n");

		Terminal::print("  Most programs will not run in this state, so it\n");
		Terminal::print("  makes sense not to even try.");
	}
	if (errno == -102) {
		Terminal::print("  Out of memory.\n\n");

		Terminal::print("  The system has run out of memory, and could not\n");
		Terminal::print("  continue execution.");
	}
	if (errno == -102) {
		Terminal::print("  Unspecified allocation failure.");
	}
	if (errno == -200) {
		Terminal::print("  GDT parameters invalid.\n\n");

		Terminal::print("  This means that the Global Descriptor Table\n");
		Terminal::print("  could not properly implement a memory map.");
	}
	if (errno == -300) {
		Terminal::print("  IDT failure.\n\n");

		Terminal::print("  Something happened that guarantees that the IDT is not working, such as a \n");
		Terminal::print("  guaranteed interrupt not occurring.");
	}
	if (errno == -400) {
		Terminal::print("  Driver failure. (not specified)\n\n");

		Terminal::print("  Something went wrong in a driver!");
	}
	if (errno == -401) {
		Terminal::print("  Driver failure. (ps2general)\n\n");

		Terminal::print("  The PS/2 Controller driver failed to perform a task.");
	}
	if (errno == -402) {
		Terminal::print("  Driver failure. (ps2keyboard)\n\n");

		Terminal::print("  The PS/2 Keyboard driver failed to perform a task.");
	}
	if (errno == -500) {
		Terminal::print("  ACPI failure.\n\n");

		Terminal::print("  Failed to find the RSDP in the ACPI.");
	}
	if (errno == -600) {
		Terminal::print("  Fault from userspace.\n\n");

		Terminal::print("  Refer to extra information.");
	}
	if (errno == -700) {
		Terminal::print("  Invalid page table index.\n\n");

		Terminal::print("  Attempted to map or access an invalid page.");
	}
	if (errno == -701) {
		Terminal::print("  Paging was initialized twice.\n\n");

		Terminal::print("  This is guaranteed to be a bug. Please report this.");
	}
	setError(1000);
}

void fault(int errno) {
	fault(errno,nullptr);
}

void warn(char* string) {
	Terminal::print("Something in the Kernel emitted a warning:\n");
	Terminal::print("  ");
	Terminal::print(string);
	Terminal::print("\n");
}




// Deprecated.

void panic(int errno,char* string) { fault(errno,string); }
// introduced in indev-2024-11-23
// deprecated in indev-2024-11-23
// superceded by fault(int, char*)

void panic(int errno) { fault(errno,nullptr); }
// introduced in indev-2024-11-23
// deprecated in indev-2024-11-23
// superceded by fault(int)
