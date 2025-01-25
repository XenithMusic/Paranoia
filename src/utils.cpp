#include <stdint.h>
#include "terminal.h"

/*

Copyright (C) 2024  XenithMusic (on github)

The Paranoia kernel is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Paranoia is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Paranoia. If not, see <https://www.gnu.org/licenses/>.

*/

extern "C" {
	void cli() {
		__asm__ __volatile__ ("cli");
	}

	void outb(uint16_t port, uint8_t val) {
		__asm__ __volatile__ ("outb %0, %1" : : "a" (val), "Nd" (port));
	}

	uint8_t inb(uint16_t port) {
		uint8_t val;
		__asm__ __volatile__ ("inb %1, %0" : "=a" (val) : "Nd" (port));
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