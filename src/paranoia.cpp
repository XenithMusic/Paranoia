#include <stdint.h>
#include "terminal.h"
#include "string.h"
#include "memory.h"
#include "const.h"
#include "types.h"
#include "fail.h"
#include "acpi.h"
#include "page.h"
#include "gdt.h"
#include "idt.h"
#include "isr.h"
#include "pit.h"

// DRIVERS

#include "drivers/ps2.h"

/*

Copyright (C) 2026  XenithMusic (on github)

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

- PS/2 Keyboard Driver
    BUG: Changing scancode sets doesn't work
    TODO: Verify that the PS/2 Controller exists.

*/

extern "C" {

    char* str; // pointer if anything demands a pointer

    void GDTinitialized(IDTEntry* idt,multiboot_info_t* mbi);    
    void PagingInitialized(IDTEntry* idt,multiboot_info_t* mbi);
    void kernel_main(multiboot_info_t* mbi) {
        // Initialize GDT as the VERY FIRST THING YOU DO.

        GDTEntry* gdt = (GDTEntry*)0x1000; // Define GDT pointer at a fixed memory location
        GDTPointer gdtpointer = initGDT(gdt);
        __attribute__((aligned(0x10))) 
        IDTEntry* idt = (IDTEntry*)((uint32_t)gdt + gdtpointer.limit + 1); // Define IDT pointer immediately following the GDT
        // Now that we have a GDT, we can continue kernel initialization.
        GDTinitialized(idt,mbi);
    }
    void GDTinitialized(IDTEntry* idt,multiboot_info_t* mbi) {
        Paging::initwkp(); // Initialize Paging as the VERY SECOND THING YOU DO.
        PagingInitialized(idt,mbi);
    }
    void PagingInitialized(IDTEntry* idt,multiboot_info_t* mbi) {
        // Set up basic environment (screen, interrupts, etc.)

        Allocator::init();

        // Backlog of pre-terminal init
        Terminal::init();
        Terminal::setCursorConfig((0xE << 4) | 0xF);
        Terminal::enableCursor();
        Terminal::print("\n:: CPU INIT\n\n");
        Terminal::print("[TIME UNKNOWN]");
            Terminal::print(" GDT initialized.");
            Terminal::print("\n");
        Terminal::print("[TIME UNKNOWN]");
            Terminal::print(" Allocator initialized.");
            Terminal::print("\n");
        Terminal::print("[TIME UNKNOWN]");
            Terminal::print(" Terminal initialized.");
            Terminal::print("\n");

        // #warning "The RSDT is being replaced with NULL; VERY dangerous, and WILL result in unintended behavior."
        // #warning "If you see this while compiling, PLEASE REPORT THIS AS A BUG."

        set_pit_count(0);

        Terminal::print("[TIME UNKNOWN]");
            Terminal::print(" PIT reset, time is measured in seconds since this message.");
            Terminal::print("\n");

        ACPITables rsdt = find_rsdt();
        // ACPITables rsdt = {};

        // if (rsdt.isValid == false) {
        //     if (rsdt.count == 0x00) {
        //         fault(-500,"Fail 0x00");
        //     } else {
        //         fault(-500,"Unknown fail.");
        //     }
        // } else {
        //     Terminal::print("RSDT is valid.");
        // }

        // INTERRUPTS

        Terminal::print("\n:: INTERRUPTS (");
            Terminal::print(parseDouble(get_pit_seconds(),str,10));
            Terminal::print(")\n\n");
        
        initIDT(idt);

        Terminal::print("[");
            Terminal::print(parseDouble(get_pit_seconds(),str,10));
            Terminal::print("] IDT initialized.");
            Terminal::print("\n");

        Terminal::print("\n:: DRIVERS (");
            Terminal::print(parseDouble(get_pit_seconds(),str,10));
            Terminal::print(")\n\n");
        PS2Returns response = ps2general::init((FADTTable*)find_sdt(&rsdt,"FACP"));
        if (response != Success) {
    //             NoPS2Controller,
    // PS2Timeout,
    // SelfTestFailure,
    // NoWorkingPorts,
    // Success,
    // NoAck,
            if (response == NoPS2Controller) {
                fault(-401,"No PS/2 controller found.)");
            } else if (response == PS2Timeout) {
                fault(-401,"Timed out while waiting for a response.");
            } else if (response == SelfTestFailure) {
                fault(-401,"PS/2 Controller reported a failure during self-test.");
            } else if (response == NoWorkingPorts) {
                fault(-401,"PS/2 Controller has no working ports.");
            } else {
                fault(-401);
            }
        }
        Terminal::print("[");
            Terminal::print(parseDouble(get_pit_seconds(),str,10));
            Terminal::print("] Initialized PS/2 Driver.");
            Terminal::print("\n");
        response = ps2keyboard::init(false);
        if (response != Success) {
            if (response == PS2Timeout) {
                fault(-402,"Timed out while waiting for a response.\n         (is the controller unresponsive?)");
            } else if (response == NoAck) {
                fault(-402,"Something should've been acknowledged, but wasn't.");
            } else {
                fault(-402);
            }
        }
        Terminal::print("[");
            Terminal::print(parseDouble(get_pit_seconds(),str,10));
            Terminal::print("] Initialized PS/2 Keyboard.");
            Terminal::print("\n");

        Terminal::print("\n:: SERVICES (");
            Terminal::print(parseDouble(get_pit_seconds(),str,10));
            Terminal::print(")\n\n");

        // no services yet.
        // - init syscalls
        // - init scheduler
        // - create first task and notify the scheduler

        Terminal::print("System information:\n");
        Terminal::print("KERNEL:               PARANOIA\n");
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
        if (CONST_DEBUGGING) {
            Terminal::print("!! WARNING: Running in debug mode, output may be far more verbose than usual!\n");
        }
        
        // // You can add more setup here (keyboard, time, etc.)

        // // approx. 0.0000326 seconds per print instruction
        // Terminal::print(parseDouble(get_pit_seconds(),str,10));
        // Terminal::print(" first call\n");
        // Terminal::print(parseDouble(get_pit_seconds(),str,10));
        // Terminal::print(" second call\n");
        // Terminal::print(parseDouble(get_pit_seconds(),str,10));
        // Terminal::print(" third call\n");
        // Terminal::print(parseDouble(get_pit_seconds(),str,10));
        // Terminal::print(" fourth call\n");

        Terminal::print("\n:: HANDOFF (");
            Terminal::print(parseDouble(get_pit_seconds(),str,10));
            Terminal::print(")\n\n");

        #ifdef CONST_DEBUGGING
        if (Paging::is_paging_enabled())
            Terminal::print("[DEBUG CHECK] Paging is enabled.\n");
        else {
            Terminal::print("[DEBUG CHECK] Paging is disabled.\n");
        }

        Terminal::printdebug("Testing near memory...\n");
        char* character = (char*)0x10000;
        character[0] = 'A';
        if (*character == 'A')
            Terminal::printdebug("[DEBUG CHECK] Succeeded.\n");

        // Terminal::printdebug("Testing far memory...\n");
        // character = (char*)0x1000000;
        // character[0] = 'A';
        // if (*character == 'A')
        //     Terminal::printdebug("[DEBUG CHECK] Succeeded. (PAGING IS DISABLED)\n");
        #endif

        double lastTick;
        double tick;

        // temporary busy-loop. handoff to the scheduler here.

        while (1) {
            tick = get_pit_seconds();
            if (lastTick+0.01 < tick) {
                lastTick = tick;
                if (ps2keyboard::keysDown[KeyCode::MODIF_ANY_CONTROL] and 
                    ps2keyboard::keysDown[KeyCode::MODIF_ANY_SHIFT] and 
                    not ps2keyboard::keysDown[KeyCode::MODIF_ANY_ALT] and 
                    not ps2keyboard::keysDown[KeyCode::MODIF_ANY_META] and 
                    ps2keyboard::keysDown[KeyCode::ALPHA_T]) {
                    Terminal::print("[");
                        Terminal::print(parseDouble(tick,str,10));
                        Terminal::print("] Keybind message print.");
                        Terminal::print("\n");
                } else {
                    const Pair<KeyCode, char>* list = codesAscii;
                    if (ps2keyboard::keysDown[KeyCode::MODIF_ANY_SHIFT]) {
                        for (const Pair<KeyCode,char> pair : codesAsciiCapital) {
                            if (ps2keyboard::keysDown[pair.first]) {
                                ps2keyboard::keysDown[pair.first] = false;
                                Terminal::print(&pair.second);
                            }
                        }
                    } else {
                        for (const Pair<KeyCode,char> pair : codesAscii) {
                            if (ps2keyboard::keysDown[pair.first]) {
                                ps2keyboard::keysDown[pair.first] = false;
                                Terminal::print(&pair.second);
                            }
                        }
                    }
                }
                ps2keyboard::processScancodes();
            }
            // if (ps2keyboard::state.state == EatingScancode)
            //     ps2keyboard::processScancodes();
            // The OS runs here
            // Terminal::swapBuffers();
        }
    }

};