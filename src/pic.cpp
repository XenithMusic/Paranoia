#include "utils.h"

/*

Copyright (C) 2026  XenithMusic (on github)

The Paranoia kernel is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Paranoia is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Paranoia. If not, see <https://www.gnu.org/licenses/>.

*/

#define PIC1		0x20		/* IO base address for master PIC */
#define PIC2		0xA0		/* IO base address for slave PIC */
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)

namespace pic
{

	void remapPIC(uint8_t PIC1_IRQ_MASK,uint8_t PIC2_IRQ_MASK) {
		outb(PIC1_COMMAND, 0x11); // starts the initialization sequence
		outb(PIC2_COMMAND, 0x11);
		outb(PIC1_DATA, 0x20); // remap offset of master PIC to 0x20 (32)
		outb(PIC2_DATA, 0x28); // remap offset of slave PIC to 0x28 (40)
		outb(PIC1_DATA, 0x04); // tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
		outb(PIC2_DATA, 0x02); // tell Slave PIC its cascade identity (0000 0010)
		outb(PIC1_DATA, 0x01); // set 8086/88 (MCS-80/85) mode
		outb(PIC2_DATA, 0x01);
		outb(PIC1_DATA, PIC1_IRQ_MASK); // mask interrupts
		outb(PIC2_DATA, PIC2_IRQ_MASK);
	}

    uint8_t in(bool isData) {
        uint16_t port = 0x20;
        if (isData) port = 0x21;
        return inb(port);
    }
    
    void out(bool isData, uint8_t data) {
        uint16_t port = 0x20;
        if (isData) port = 0x21;
        outb(port,data);
    }
    
    uint8_t inblocking(bool isData, int timeout = 0) {
        if (timeout == -1) {
            while ((in(false) & 0x01) == 0);
        } else {
            while ((in(false) & 0x01) == 0 and timeout >= 0) timeout--;
        }
        return in(isData);
    }
    
    bool outack(bool isData, uint8_t data) {
        out(isData,data);
        uint8_t ack = inblocking(true);
        return ack == 0xFA;
    }
    
    void mask(uint8_t ands,uint8_t ors) {
        uint8_t mask = in(true);
        mask &= ands;
        mask |= ors;
        out(true,mask);
    }
}