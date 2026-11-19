#include "memory.h"
#include <stdint.h>
#include "isrs.h"
#include "types.h"
#include "serial.h"
#include "utils.h"
#include "pic.h"
#include "circqueue.h"
#include "drivers/ps2.h"

#define PIC1		0x20		/* IO base address for master PIC */
#define PIC2		0xA0		/* IO base address for slave PIC */
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)

#define IDT_ENTRIES 256
#define IDT_TABLE_SIZE sizeof(void*)*IDT_ENTRIES
#define IDT_ENTRIES_SIZE sizeof(IDTEntry)*IDT_ENTRIES

namespace IDT {
    char str1[256];
    char str2[256];
    static void** table;
    IDTEntry* entries;
    static bool userspaces[IDT_ENTRIES];
    static bool deferred[IDT_ENTRIES];
    static CircQueue<IDTDispatchArgs> queue;
    void eoi(uint8_t irq) {
        if (irq >= 8) {
            outb(PIC2_COMMAND,0x20);
        }
        outb(PIC1_COMMAND, 0x20);
    }
	void remapPIC() {
		// optionally, clear data registers (mask all IRQs for now)
		// doing this manually lol
		uint8_t PIC1_IRQ_MASK = 0xFF;
		// PIC1_IRQ_MASK &= ~(1 << 0); // unmask IRQ0
		PIC1_IRQ_MASK &= ~(1 << 1); // unmask IRQ1
		uint8_t PIC2_IRQ_MASK = 0xFF;
		pic::remapPIC(PIC1_IRQ_MASK,PIC2_IRQ_MASK);
	}
    void init() {
        cli();
        queue.init(64);
        table = (void**)Allocator::kalloc(IDT_TABLE_SIZE);
        kmemset(table, 0, IDT_TABLE_SIZE);
        entries = (IDTEntry*)Allocator::kalloc(IDT_ENTRIES_SIZE);
    }
    void genericFaultHandler(int index,InterruptFrame* frame, uint32_t errorCode) {
        fault(1,"Refer to the `source` text for the exception index.",parseInt(index, 10));
    }
    void generalProtection(int index,InterruptFrame* frame, uint32_t errorCode) {
        fault(-1100,parseU32(frame->cs, 16));
    }
    void pageFault(int index,InterruptFrame* frame, uint32_t errorCode) {
        uint32_t cr2 = 0;
        __asm__ volatile ("mov %%cr2, %0" : "=r"(cr2));
        fault(-702,parseU32(errorCode,str1,16),parseU32(frame->cs,str2,16));
    }
    void harshFaultHandler(int index,InterruptFrame* frame, uint32_t errorCode) {
        warn("Critical fault! (index ");
        warn(parseInt(index, 10));
        warn(")");
        induceHalt();
    }
    void breakpointHandler(int index,InterruptFrame* frame, uint32_t errorCode) {
        cli();
        Serial::print("breakpoint handler");
        __asm__ __volatile__("hlt"); // stop the state as soon as possible to preserve it
    }
    void irq1Handler(int index,InterruptFrame* frame, uint32_t errorCode) {
        Serial::print("IRQ1");
        uint8_t status = inb(0x64);
        bool obf = status & (1 << 0);
        bool ismouse = status & (1 << 5); // HACK: Supposedly, bit 5 is inconsistent on very old hardware during controller transitions.
        if (not obf // Protects against Surpious IRQ1
            or ismouse // Protects against Mouse Leakage
        ) {
            eoi(1);
            return;
        }
        uint8_t scancode = inb(0x60);
        auto &st = ps2keyboard::state;
        if (st.state == WaitingForScancodes or st.state == EatingScancode) {
            st.state = EatingScancode;
            st.data[st.lastScancode] = scancode;
            st.lastScancode = (st.lastScancode+1)%64;
        } else if (st.state == WaitingForAck) {
            if (scancode == 0xFA) {
                st.state = NoProcess;
                st.data[0] = scancode; // BUG: This overwrites state.data.
            } else if (scancode == 0xFE) {
                st.state = Failure;
            }
        }
        eoi(1);
        return;
    }
    void add(uint8_t index, void* handler) {
        table[index] = handler;
    }
    void addDefaults() {
        add(0,(void*)&genericFaultHandler);
        add(3,(void*)&breakpointHandler);
        add(4,(void*)&genericFaultHandler);
        add(5,(void*)&genericFaultHandler);
        add(6,(void*)&genericFaultHandler);
        add(7,(void*)&genericFaultHandler);
        add(8,(void*)&breakpointHandler);
        add(10,(void*)&genericFaultHandler);
        add(11,(void*)&genericFaultHandler);
        add(12,(void*)&genericFaultHandler);
        add(13,(void*)&generalProtection);
        add(14,(void*)&pageFault);
        add(16,(void*)&genericFaultHandler);
        add(17,(void*)&genericFaultHandler);
        add(18,(void*)&harshFaultHandler);
        add(19,(void*)&genericFaultHandler);
        
        // IRQ
        add(0x21,(void*)&irq1Handler);
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
        remapPIC();
        sti();
    }
    void dispatch(IDTDispatchArgs args) {
        int index = args.index;
        InterruptFrame* frame = args.frame;
        uint32_t errorCode = args.errorCode;
        if (table[index] == nullptr) {
            warn("Unimplemented interrupt handler; this may cause a double (or triple!) fault. (index = ");
            warn(parseInt(index, 10));
            warn(")\n");
            return;
        }
        using Handler = void(*)(int,InterruptFrame*,uint32_t);
        Handler fn = (Handler)(table[index]);
        fn(index,frame,errorCode);
    }
    void dispatch(int index, InterruptFrame* frame, uint32_t errorCode) {
        if (deferred[index]) {
            queue.write({index,frame,errorCode});
            return;
        }
        dispatch({index,frame,errorCode});
    }
    void handleQueue() {
        while (true) {
            Optional<IDTDispatchArgs> opt = queue.read();
            if (!opt.present) break;
            IDTDispatchArgs val = opt.value;
            dispatch(val);
        }
    }
    void setUserspace(int index, bool state) {userspaces[index] = state;}
    void setDeferred(int index, bool state) {deferred[index] = state;}
}