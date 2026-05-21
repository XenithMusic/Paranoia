#include "fail.h"
#include "types.h"
#include "paranoia.h"
#include "utils.h"

/*

Copyright (C) 2026  XenithMusic (on github)

The Paranoia kernel is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Paranoia is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Paranoia. If not, see <https://www.gnu.org/licenses/>.

*/

// void encodeGdtEntry(uint8_t *target, struct GDT source)
// {
//     // Check the limit to make sure that it can be encoded
//     if (source.limit > 0xFFFFF) {
//         fault(-200,"GDT cannot encode limits larger than 0xFFFFF");
//     }
    
//     // Encode the limit
//     target[0] = source.limit & 0xFF;
//     target[1] = (source.limit >> 8) & 0xFF;
//     target[6] = (source.limit >> 16) & 0x0F;
    
//     // Encode the base
//     target[2] = source.base & 0xFF;
//     target[3] = (source.base >> 8) & 0xFF;
//     target[4] = (source.base >> 16) & 0xFF;
//     target[7] = (source.base >> 24) & 0xFF;
    
//     // Encode the access byte
//     target[5] = source.access_byte;
    
//     // Encode the flags
//     target[6] |= (source.flags << 4);
// }

extern "C" {
    GDTPointer gdtptr __attribute__((aligned(4)));
    void setGDT(uint16_t limit, void* base);
    void encodeGDT(GDTEntry* gdt, int index, struct GDTEntry entry) {
        gdt[index] = entry;
    }

    void GDTPartTwo(GDTEntry* gdt) {
        __asm__ __volatile__ (
            "mov $0x10, %eax\n"
            "mov %eax, %ds\n"
            "mov %eax, %es\n"
            "mov %eax, %fs\n"
            "mov %eax, %gs\n"
            "mov %eax, %ss\n"
            "mov $0x08, %eax\n"
        );
        return;
    }
    GDTPointer initGDT(GDTEntry* gdt, TSS32* tss) {
        cli();
        encodeGDT(gdt, 0,{0,0,0,0,0,0});
        encodeGDT(gdt, 1,{0xFFFF,0,0,0x9A,0xCF,0}); // Kernel Code segment (0x08)
        encodeGDT(gdt, 2,{0xFFFF,0,0,0x92,0xCF,0}); // Kernel Data segment (0x10)
        encodeGDT(gdt, 3,{0xFFFF,0,0,0xFA,0xCF,0}); // User Code segment (0x18)
        encodeGDT(gdt, 4,{0xFFFF,0,0,0xF2,0xCF,0}); // User Data segment (0x20)
        uint32_t tss_addr = (uint32_t)&tss;
        uint32_t tss_limit = sizeof(tss) - 1;
        GDTEntry tss_entry = {};
        tss_entry.limit_low = tss_limit&0xFFFF;
        tss_entry.base_low = tss_addr&0xFFFF;
        tss_entry.base_middle = (tss_addr >> 16)&0xFF;
        tss_entry.access = 0x89;
        tss_entry.granularity = (tss_limit >> 16)&0x0F;
        tss_entry.base_high = (tss_addr >> 24)&0xFF;
        encodeGDT(gdt, 5, tss_entry); // TSS segment (0x28)
        gdtptr.limit = sizeof(GDTEntry) * 6 - 1;
        gdtptr.base = (uint32_t)gdt;

        // do TSS stuff... >:c
        tss->ss0 = 0x10; // kernel data seg
        
        __asm__ __volatile__ ("lgdt (%0)" : : "r"(&gdtptr)); // Load GDT
        __asm__ __volatile__ (
            "ljmp $0x08, $next_label\n" // Far jump to reload CS
            "next_label:\n"
            "mov $0x28, %%ax\n"
            "ltr %%ax\n" // ALERT: untested. if there are crashes in either userspace or during GDT init, check here first!
            ::: "ax", "memory"
        );
        // // Update segment registers after far jump
        // __asm__ __volatile__ (
        //     "mov $0x10, %ax\n" // Kernel Data segment selector (index 2)
        //     "mov %ax, %ds\n"
        //     "mov %ax, %es\n"
        //     "mov %ax, %fs\n"
        //     "mov %ax, %gs\n"
        //     "mov %ax, %ss\n"
        // );
        GDTPartTwo(gdt);
        return gdtptr;
    }
    void switchUserspace() {
        __asm__ __volatile__ (
            "mov $0x13, %eax\n" // User data segment selector (index 4)
            "mov %eax, %ds\n"
            "mov %eax, %es\n"
            "mov %eax, %fs\n"
            "mov %eax, %gs\n"
            "mov %eax, %ss\n"
            "jmp $0x0B, $.flushSWITCHUSERSPACE\n" // User code segment selector (index 3)
            ".flushSWITCHUSERSPACE:\n"
        );
        return;
    }
}