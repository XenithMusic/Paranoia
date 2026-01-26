#include "terminal.h"
#include "string.h"
#include "pit.h"
#include "ps2.h"
#include "acpi.h"
#include "fail.h"

/*

Copyright (C) 2026  XenithMusic (on github)

The Paranoia kernel is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Paranoia is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Paranoia. If not, see <https://www.gnu.org/licenses/>.

*/

enum DriverReturn {
    SUCCESS,
    FAILURE
};

char* throwawayString;

namespace drivermanager {
    DriverReturn init(ACPITables* rsdt) {
        Terminal::print("[drivermanager::init @ ");
            Terminal::print(parseDouble(get_pit_seconds(),throwawayString,10));
            Terminal::print("] Initializing mandatory drivers...\n\n");
        PS2Returns response = ps2general::init((FADTTable*)find_sdt(rsdt,"FACP"));
        if (response != Success) {
            if (response == NoPS2Controller) {
                fault(-403,"No PS/2 controller found.","PS/2 Controller");
            } else if (response == PS2Timeout) {
                fault(-403,"Timed out while waiting for a response.","PS/2 Controller");
            } else if (response == SelfTestFailure) {
                fault(-403,"Controller Self-Test failure.","PS/2 Controller");
            } else if (response == NoWorkingPorts) {
                fault(-403,"No working ports.","PS/2 Controller");
            } else {
                fault(-403,nullptr,"PS/2 Controller");
            }
        }
        Terminal::print("[");
            Terminal::print(parseDouble(get_pit_seconds(),throwawayString,10));
            Terminal::print("] Initialized PS/2 Driver.");
            Terminal::print("\n");
        
        
        response = ps2keyboard::init(false);
        if (response != Success) {
            if (response == PS2Timeout) {
                fault(-403,"Timed out while waiting for a response.\n         (is the controller unresponsive?)","PS/2 Keyboard");
            } else if (response == NoAck) {
                fault(-403,"Something should've been acknowledged, but wasn't.","PS/2 Keyboard");
            } else if (response == Failure) {
                fault(-403,"Something went wrong.","PS/2 Keyboard");
            } else {
                fault(-403,nullptr,"PS/2 Controller");
            }
        }
        Terminal::print("[");
            Terminal::print(parseDouble(get_pit_seconds(),throwawayString,10));
            Terminal::print("] Initialized PS/2 Keyboard.");
            Terminal::print("\n");
        return SUCCESS;
    }
}

namespace drivers {
}