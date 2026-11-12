#include "memory.h"
#include <stdint.h>
#include "isrs.h"
#include "types.h"

#define IDT_ENTRIES 256
#define IDT_TABLE_SIZE sizeof(void*)*IDT_ENTRIES
#define IDT_ENTRIES_SIZE sizeof(IDTEntry)*IDT_ENTRIES

namespace IDT {
    static void** table;
    IDTEntry* entries;
    static bool userspaces[IDT_ENTRIES];
    void init() {
        table = (void**)Allocator::kalloc(IDT_TABLE_SIZE);
        kmemset(table, 0, IDT_TABLE_SIZE);
        entries = (IDTEntry*)Allocator::kalloc(IDT_ENTRIES_SIZE);
    }
    void add(uint8_t index, void* handler) {
        table[index] = handler;
    }
    void finish() {
        size_t i = 0;
        for (uintptr_t ptr : isr_stub_table) {
            entries[i] = IDTEntry{(uint16_t)(ptr),0x08,0x00,0x8E,(uint16_t)(ptr>>16)};
            if (userspaces[i]) {
                entries[i].type_attr = 0xEE;
            }
            i++;
        }
        IDTPointer idtr;
        idtr.base = (uint32_t)entries;
        idtr.limit = IDT_ENTRIES_SIZE-1;
        __asm__ __volatile__ ("lidt %0" : : "m" (idtr));
    }
    void dispatch(int index, InterruptFrame* frame, uint32_t errorCode) {
        if (table[index] == nullptr) {
            warn("Unimplemented interrupt handler; this may cause a double (or triple!) fault. (index = ");
            warn(parseInt(index, 10));
            warn(")\n");
            return;
        }
        using Handler = void(*)(InterruptFrame*,uint32_t);
        Handler fn = (Handler)(table[index]);
        fn(frame,errorCode);
    }
    void setUserspace(int index, bool state) {userspaces[index] = state;}
}