#include <stdint.h>
#include "terminal.h"
#include "string.h"
#include "const.h"
#include "pit.h"

extern "C" {
    
    void kernel_main(void) {
        // Set up basic environment (screen, interrupts, etc.)
        Terminal::init();
        Terminal::print("System information:\n");
        Terminal::print("KERNEL:             PARANOIA\n");
        Terminal::print("- VERSION:            ");
            Terminal::print(CONST_VERSION);
            Terminal::print("\n");
        Terminal::print("- COMPILATION DATE:   ");
            Terminal::print(CONST_COMPDATE);
            Terminal::print("\n");

        // You can add more setup here (keyboard, time, etc.)
        
        // Now enter an infinite loop (just to keep the OS running)

        char* str;
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

            if (lastTick+0.2 < tick) {
                lastTick = tick;
                Terminal::print(parseDouble(tick,str,10));
                Terminal::print("\n");
            }
            // The OS runs here
            // Terminal::swapBuffers();
        }
    }

};