#include <stdint.h>
#include <stddef.h>
#include "terminal.h"

/*

Copyright (C) 2026  XenithMusic (on github)

The Paranoia kernel is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Paranoia is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Paranoia. If not, see <https://www.gnu.org/licenses/>.

*/

extern "C" {
	void cli() { // disable interrupts
		__asm__ __volatile__ ("cli");
	}

	void sti() { // enable interrupts
		__asm__ __volatile__ ("sti");
	}

	bool checkInterrupts() {
		uint32_t flags;
		asm volatile (
			"pushf\n\t"    // push EFLAGS onto stack
			"pop %0"       // pop into flags variable
			: "=r"(flags)  // output
			:              // no input
			:              // no clobbers
		);
		return (flags & (1 << 9)) > 0;
	}

	void outb(uint16_t port, uint8_t val) {
		__asm__ __volatile__ ("outb %0, %1" : : "a" (val), "Nd" (port));
	}

	void outw(uint16_t port, uint16_t val) {
		__asm__ __volatile__ ("outw %0, %1" : : "a" (val), "Nd" (port));
	}

	void outsw(uint16_t port, void* buffer, size_t count) {
		__asm__ __volatile__(
			"rep outsw"
			: "+S"(buffer), "+c"(count)
			: "d"(port)
		);
	}

	void io_wait() {
		// dummy i/o operation
		outb(0x80, 0);
	}

	void interrupt(uint8_t vector) {
		switch (vector) {
			case 0x20: __asm__ __volatile__("int $0x20"); break;
			case 0x21: __asm__ __volatile__("int $0x21"); break;
			case 0x80: __asm__ __volatile__("int $0x80"); break;
			default: /* unsupported */ break;
		}
	}

	uint8_t inb(uint16_t port) {
		uint8_t val;
		__asm__ __volatile__ ("inb %1, %0" : "=a" (val) : "Nd" (port));
		return val;
	}

	uint16_t inw(uint16_t port) {
		uint16_t val;
		__asm__ __volatile__ ("inw %1, %0" : "=a" (val) : "Nd" (port));
		return val;
	}
	void induceHalt();
	void induceHang() {
		Terminal::print("\n\n( System hang induced. )\n");
		cli();
		while (true) {};
		Terminal::print("\n\nSystem escaped from hang state, halting.");
		induceHalt();
	}
	void induceHalt() {
		Terminal::print("\n\n( System halted. )\n");
		__asm__ __volatile__ ("hlt");
		Terminal::print("\n\nSystem escaped from halt state, hanging.");
		induceHang();
	}
}