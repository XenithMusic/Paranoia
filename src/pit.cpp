#include "assemblyUtils.h"
#include "math.h"

/*

Copyright (C) 2024  XenithMusic (on github)

The Paranoia kernel is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Paranoia is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Paranoia. If not, see <https://www.gnu.org/licenses/>.

*/


extern "C" {
	unsigned previous_count;
	uint64_t overflowCount;

	void set_pit_count(int count) {
		overflowCount = 0;
		previous_count = count%65536;
		// Disable interrupts
		cli();
		
		// Set low byte
		outb(0x40,count&0xFF);		// Low byte
		outb(0x40,(count&0xFF00)>>8);	// High byte
		return;
	}
	uint64_t get_pit_count(void) {
		unsigned count = 0;
		
		// Disable interrupts
		cli();
		
		// al = channel in bits 6 and 7, remaining bits clear
		outb(0x43,0b0000000);
		
		count = inb(0x40);		// Low byte
		count |= inb(0x40)<<8;		// High byte

		count = 65535-count;

		if (count < previous_count) {
			overflowCount++;
		}
		previous_count = count;
		
		return count+(overflowCount*65536);
	}

	double get_pit_seconds(void) {
		uint32_t count = get_pit_count();
		double seconds = count / 1193182.0;
		return seconds;
	}

	void sleep(double seconds) {
		double now = get_pit_seconds();
		while (get_pit_seconds() < now+seconds) {};
		return;
	}
}