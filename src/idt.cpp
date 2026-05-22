#include <stdint.h>
#include <stddef.h>
#include "types.h"
#include "utils.h"
#include "isr.h"
#include "pic.h"

/*

Copyright (C) 2026  XenithMusic (on github)

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
		// optionally, clear data registers (mask all IRQs for now)
		// doing this manually lol
		uint8_t PIC1_IRQ_MASK = 0xFF;
		PIC1_IRQ_MASK &= ~(1 << 0); // unmask IRQ0
		PIC1_IRQ_MASK &= ~(1 << 1); // unmask IRQ1
		uint8_t PIC2_IRQ_MASK = 0xFF;
		pic::remapPIC(PIC1_IRQ_MASK,PIC2_IRQ_MASK);
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
		encodeIDT(idt,0x0d,(void*)&genprotfault_handler,0x8e);
		encodeIDT(idt,0x20,(void*)&basic_eoi_assembly_low,0x8e);
		encodeIDT(idt,0x21,(void*)&irq1_assembly,0x8e);
		encodeIDT(idt,0x22,(void*)&basic_eoi_assembly_low,0x8e);
		encodeIDT(idt,0x23,(void*)&basic_eoi_assembly_low,0x8e);
		encodeIDT(idt,0x24,(void*)&basic_eoi_assembly_low,0x8e);
		encodeIDT(idt,0x25,(void*)&basic_eoi_assembly_low,0x8e);
		encodeIDT(idt,0x26,(void*)&basic_eoi_assembly_low,0x8e);
		encodeIDT(idt,0x27,(void*)&basic_eoi_assembly_low,0x8e);
		encodeIDT(idt,0x28,(void*)&basic_eoi_assembly_high,0x8e);
		encodeIDT(idt,0x29,(void*)&basic_eoi_assembly_high,0x8e);
		encodeIDT(idt,0x2a,(void*)&basic_eoi_assembly_high,0x8e);
		encodeIDT(idt,0x2b,(void*)&basic_eoi_assembly_high,0x8e);
		encodeIDT(idt,0x2c,(void*)&basic_eoi_assembly_high,0x8e);
		encodeIDT(idt,0x2d,(void*)&basic_eoi_assembly_high,0x8e);
		encodeIDT(idt,0x2e,(void*)&basic_eoi_assembly_high,0x8e);
		encodeIDT(idt,0x2f,(void*)&basic_eoi_assembly_high,0x8e);
		lidt(idt);
		sti();
	}
}