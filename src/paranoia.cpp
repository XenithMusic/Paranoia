#include <stdint.h>
#include "terminal.h"
#include "string.h"
#include "const.h"
#include "pit.h"
#include "memory.h"
#include "types.h"
#include "fail.h"
#include "gdt.h"
#include "idt.h"
#include "isr.h"

/*

Copyright (C) 2024  XenithMusic (on github)

The Paranoia kernel is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Paranoia is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Paranoia. If not, see <https://www.gnu.org/licenses/>.

*/

/*

TODO:
- Memory Management

    [X] Basic kmalloc
        [X] Allocate
    [X] Basic kfree

- Interrupt Descriptor Table
    [X] Make space for the IDT
    [X] Tell the CPU where that is
    [X] Tell the PIC to screw off with BIOS defaults (https://wiki.osdev.org/PIC#Programming_the_PIC_chips)
    [X] Write a couple of ISR (interrupt service routines) for IRQs and exceptions
            -- i have exceptions right now --
    [X] Put the addresses of the ISR handlers in the appropiate descriptors (in the IDT)

*/

extern "C" {

    char* str; // pointer if anything demands a pointer

    void GDTinitialized(IDTEntry* idt);    
    void kernel_main(multiboot_info* mbi) {
        // Initialize GDT as the VERY FIRST THING YOU DO.

        GDTEntry* gdt = (GDTEntry*)0x1000; // Define GDT pointer at a fixed memory location
        GDTPointer gdtpointer = initGDT(gdt);
        __attribute__((aligned(0x10))) 
        IDTEntry* idt = (IDTEntry*)((uint32_t)gdt + gdtpointer.limit + 1); // Define IDT pointer immediately following the GDT
        // Now that we have a GDT, we can continue kernel initialization.
        GDTinitialized(idt);
    }
    void GDTinitialized(IDTEntry* idt) {
        // Set up basic environment (screen, interrupts, etc.)

        Allocator::init();

        // fault(69420,"On-boot FAULT test.");

        // Backlog of pre-terminal init
        Terminal::init();
        Terminal::print("[TIME UNKNOWN]");
            Terminal::print(" GDT initialized.");
            Terminal::print("\n");
        Terminal::print("[TIME UNKNOWN]");
            Terminal::print(" Allocator initialized.");
            Terminal::print("\n");
        Terminal::print("[TIME UNKNOWN]");
            Terminal::print(" Terminal initialized.");
            Terminal::print("\n");

        set_pit_count(0);

        Terminal::print("[TIME UNKNOWN]");
            Terminal::print(" PIT reset, time is measured since this message.\n");
            Terminal::print("\n");

        // Post-terminal init
        
        initIDT(idt);

        Terminal::print("[");
            Terminal::print(parseDouble(get_pit_seconds(),str,10));
            Terminal::print("] IDT initialized.\n");
            Terminal::print("\n");

        Terminal::print("System information:\n");
        Terminal::print("KERNEL:             PARANOIA\n");
        Terminal::print("- VERSION:            ");
            Terminal::print(CONST_VERSION);
            Terminal::print("\n");
        Terminal::print("- COMPILATION DATE:   ");
            Terminal::print(CONST_COMPDATE);
            Terminal::print("\n");
        Terminal::print("- AUTHORED BY:        ");
            Terminal::print(CONST_AUTHOR);
            Terminal::print("\n");
        Terminal::print("- KERNEL SIZE:        ");
            Terminal::print(parseDouble((double)CONST_KERNELSIZE/4/1024,str,2));
            Terminal::print("MiB\n");
        
        // You can add more setup here (keyboard, time, etc.)

        // approx. 0.0000326 seconds per print instruction
        Terminal::print(parseDouble(get_pit_seconds(),str,10));
        Terminal::print(" first call\n");
        Terminal::print(parseDouble(get_pit_seconds(),str,10));
        Terminal::print(" second call\n");
        Terminal::print(parseDouble(get_pit_seconds(),str,10));
        Terminal::print(" third call\n");
        Terminal::print(parseDouble(get_pit_seconds(),str,10));
        Terminal::print(" fourth call\n");

        double lastTick;
        double tick;

        while (1) {

            tick = get_pit_seconds();

            if (lastTick+1 < tick) {
                lastTick = tick;
                Terminal::print(parseDouble(tick,str,10));
                Terminal::print("\n");
            }
            // The OS runs here
            // Terminal::swapBuffers();
        }
    }

};