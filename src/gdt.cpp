#include "fail.h"
#include "types.h"
#include "paranoia.h"
#include "utils.h"

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
    GDTPointer initGDT(GDTEntry* gdt) {
        cli();
        encodeGDT(gdt, 0,{0,0,0,0,0,0});
        encodeGDT(gdt, 1,{0xFFFF,0,0,0x9A,0xCF,0}); // Kernel Code segment (0x08)
        encodeGDT(gdt, 2,{0xFFFF,0,0,0x92,0xCF,0}); // Kernel Data segment (0x10)
        encodeGDT(gdt, 3,{0xFFFF,0,0,0xFA,0xCF,0}); // User Code segment (0x18)
        encodeGDT(gdt, 4,{0xFFFF,0,0,0xF2,0xCF,0}); // User Data segment (0x20)
        gdtptr.limit = sizeof(GDTEntry) * 5 - 1;
        gdtptr.base = (uint32_t)gdt;
        __asm__ __volatile__ ("lgdt (%0)" : : "r"(&gdtptr)); // Load GDT
        __asm__ __volatile__ (
            "ljmp $0x08, $next_label\n" // Far jump to reload CS
            "next_label:\n"
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