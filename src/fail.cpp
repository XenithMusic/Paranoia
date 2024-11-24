#include "terminal.h"
#include "string.h"
#include "pit.h"

int error;

void setError(int errno) {
	if (errno == 1000) {
		Terminal::print("System halted.\n");
		while (true) {};
	} else {
		error = errno;
	}
}

int getError() {
	return error;
}

void clearError() {
	error = 0;
}

void fault(int errno,char* string) {
	char* str;
	Terminal::init();
	Terminal::clearScreen();
	Terminal::setRow(0);
	Terminal::print("A significant error occurred in the Paranoia kernel,\n");
		Terminal::print("and it needed to stop executing immediately.\n\n");
	Terminal::print("FAULT INFO\n");
	Terminal::print("  Code:  ");
		Terminal::print(parseInt(errno,str,10));
		Terminal::print("\n");
	if (string != nullptr) {
	Terminal::print("  Extra: ");
		Terminal::print(string);
		Terminal::print("\n");
	}
	Terminal::print("\n");
	Terminal::print("Your computer will not restart itself.\n\n");
	Terminal::print("Panic specific details:\n");
	if (errno == 69420) {
		Terminal::print("  This is an errno intended only for testing\n");
		Terminal::print("  the kernel fault system.\n\n");
		Terminal::print("  If this occurs in a production release, please\n");
		Terminal::print("  inform the developer at <xenith.contact.mail@gm\n");
		Terminal::print("  ail.com>, and then downgrade.\n\n");
	}
	if (errno == -100) {
		Terminal::print("  A segmented memory block did not end.\n\n");

		Terminal::print("  Memory integrity cannot be verified, when a\n");
		Terminal::print("  property of all memory blocks does not exist.\n\n");
	}
	if (errno == -101) {
		Terminal::print("  The memory allocator failed to initialize.\n\n");

		Terminal::print("  Most programs will not run in this state, so it\n");
		Terminal::print("  makes sense not to even try.\n\n");
	}
	setError(1000);
}

void fault(int errno) {
	fault(errno,nullptr);
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
