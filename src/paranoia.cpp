#include <stdint.h>
#include "terminal.h"
#include "string.h"
#include "const.h"
#include "pit.h"
#include "memory.h"
#include "types.h"

/*

TODO:
- Memory Management
    [X] Parse MMAP table
    [X] Basic kmalloc
        [X] Allocate
        [ ] Split memory when needed so you don't allocate lots of memory unnecessarily!
    [ ] Basic kfree
- Interrupt Descriptor Table
    [ ] Make space for the IDT (figure out memory management first)
    [ ] Tell the CPU where that is
    [ ] Tell the PIC to screw off with BIOS defaults (https://wiki.osdev.org/PIC#Programming_the_PIC_chips)
    [ ] Write a couple of ISR (interrupt service routines) for IRQs and exceptions
    [ ] Put the addresses of the ISR handlers in the appropiate descriptors (in the IDT)
    [ ] Enable all supported interrupts in the IRQ mask of the PIC

*/

extern "C" {
    
    void kernel_main(multiboot_info* mbi) {
        // Set up basic environment (screen, interrupts, etc.)

        char* str; // pointer if anything demands a pointer

        Allocator::init(mbi);
        Terminal::init();
        Terminal::print(parseU32((mbi->mmap_length),str,10));
        Terminal::print(" bytes\n");
        Terminal::print(parseU32((mbi->mmap_addr),str,10));
        Terminal::print(" bytes\n");
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

        // You can add more setup here (keyboard, time, etc.)
        
        // Now enter an infinite loop (just to keep the OS running)
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