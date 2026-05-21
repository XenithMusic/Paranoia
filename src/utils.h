#include <stdint.h>
#include <stddef.h>

/*

Copyright (C) 2026  XenithMusic (on github)

The Paranoia kernel is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Paranoia is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Paranoia. If not, see <https://www.gnu.org/licenses/>.

*/

extern "C" {
	void cli();
	void sti();
	bool checkInterrupts();
	void io_wait();

	void interrupt(uint8_t num);

	void outb(uint16_t port, uint8_t val);
	uint8_t inb(uint16_t port);
	void outw(uint16_t port, uint16_t val);
	void outstr(uint16_t port, char* val);
	void outsw(uint16_t port, void* buffer, size_t count);
	uint16_t inw(uint16_t port);

	void interrupt(uint8_t vector);

	void induceHang();
	void induceHalt();
}