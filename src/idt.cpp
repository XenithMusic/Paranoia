#include <stdint.h>
#include <stddef.h>
#include "types.h"
#include "utils.h"
#include "isr.h"

#define PIC1		0x20		/* IO base address for master PIC */
#define PIC2		0xA0		/* IO base address for slave PIC */
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)

/*

Copyright (C) 2024  XenithMusic (on github)

The Paranoia kernel is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Paranoia is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Paranoia. If not, see <https://www.gnu.org/licenses/>.

*/

extern "C" void setIDT(uint16_t limit, void* base); // defined in assembly
// deprecated MAY 04 2025
// setIDT is misleading when you look at the gdt code; it does not set an IDT entry, it loads the IDT.
// sorry for the confusion!

static IDTPointer idtr;
uint8_t IDT_MAX_DESCRIPTORS = 0;

extern "C" {
	void remapPIC() {
		outb(PIC1_COMMAND, 0x11); // starts the initialization sequence
		outb(PIC2_COMMAND, 0x11);
		outb(PIC1_DATA, 0x20); // remap offset of master PIC to 0x20 (32)
		outb(PIC2_DATA, 0x28); // remap offset of slave PIC to 0x28 (40)
		outb(PIC1_DATA, 0x04); // tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
		outb(PIC2_DATA, 0x02); // tell Slave PIC its cascade identity (0000 0010)
		outb(PIC1_DATA, 0x01); // set 8086/88 (MCS-80/85) mode
		outb(PIC2_DATA, 0x01);
		// optionally, clear data registers (mask all IRQs for now)
		outb(PIC1_DATA, 0x0);
		outb(PIC2_DATA, 0x0);
	}
	void catchAll() { // generic thing to see if any interrupts occur (do not replace)
		while (1) __asm__ __volatile__ ("hlt");
	}
	void encodeIDT(IDTEntry* idt, uint8_t vector, void* isr, uint8_t flags) {
		IDTEntry* descriptor = &idt[vector];
		descriptor->offset_low = (uint32_t)isr & 0xFFFF;
		descriptor->selector = 0x08;
		descriptor->type_attr = flags;
		descriptor->offset_high = (uint32_t)isr >> 16;
		descriptor->zero = 0;
		if (vector+1 > IDT_MAX_DESCRIPTORS) IDT_MAX_DESCRIPTORS = vector+1;
	}
	void lidt(IDTEntry* idt) {
		idtr.base = (uintptr_t)&idt[0];
		idtr.limit = (uint16_t)sizeof(IDTEntry)*IDT_MAX_DESCRIPTORS - 1;
		__asm__ __volatile__ ("lidt %0" : : "m"(idtr));
	}
	void initIDT(IDTEntry* idt) {
		cli(); // Redundant, but worth it; weird bugs are not fun.
		remapPIC();
		// interrupts for exceptions
		for (int vec = 0; vec < 32; vec++) {
			encodeIDT(idt,vec,(void*)&exception_handler,0x8e);
		}
		encodeIDT(idt,0x00,(void*)&divzero_handler,0x8e);
		encodeIDT(idt,0x21,(void*)&irq1_handler,0x8e);
		lidt(idt);
		sti();
	}
}