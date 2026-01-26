#include "terminal.h"
#include "string.h"
#include "acpi.h"
#include "fail.h"
#include "pit.h"
#include "ps2.h"
#include "ata.h"

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
        PS2Returns response_ps2 = ps2general::init((FADTTable*)find_sdt(rsdt,"FACP"));
        if (response_ps2 != Success) {
            if (response_ps2 == NoPS2Controller) {
                fault(-403,"No PS/2 controller found.","PS/2 Controller");
            } else if (response_ps2 == PS2Timeout) {
                fault(-403,"Timed out while waiting for a response.","PS/2 Controller");
            } else if (response_ps2 == SelfTestFailure) {
                fault(-403,"Controller Self-Test failure.","PS/2 Controller");
            } else if (response_ps2 == NoWorkingPorts) {
                fault(-403,"No working ports.","PS/2 Controller");
            } else {
                fault(-403,nullptr,"PS/2 Controller");
            }
        }
        Terminal::print("[");
            Terminal::print(parseDouble(get_pit_seconds(),throwawayString,10));
            Terminal::print("] Initialized PS/2 Driver.");
            Terminal::print("\n");
        
        
        response_ps2 = ps2keyboard::init(false);
        if (response_ps2 != Success) {
            if (response_ps2 == PS2Timeout) {
                fault(-403,"Timed out while waiting for a response.\n         (is the controller unresponsive?)","PS/2 Keyboard");
            } else if (response_ps2 == NoAck) {
                fault(-403,"Something should've been acknowledged, but wasn't.","PS/2 Keyboard");
            } else {
                fault(-403,nullptr,"PS/2 Controller");
            }
        }
        Terminal::print("[");
            Terminal::print(parseDouble(get_pit_seconds(),throwawayString,10));
            Terminal::print("] Initialized PS/2 Keyboard.");
            Terminal::print("\n");
        
        disk_ata::ATAState response_ata = disk_ata::init();
        if (response_ata != disk_ata::SUCCESS) {
            if (response_ata == disk_ata::OUT_OF_BOUNDS) {
                fault(-403,"Out of bounds.","ATA Hard Drive");
            } else {
                fault(-403,nullptr,"ATA Hard Drive");
            }
        }
        Terminal::print("[");
            Terminal::print(parseDouble(get_pit_seconds(),throwawayString,10));
            Terminal::print("] Initialized ATA Driver.");
            Terminal::print("\n");
        Terminal::print("ATA read demo: ");
        response_ata = disk_ata::read_sector(0xF000000/512,disk_ata::ATADrive::MASTER);
        if (response_ata != disk_ata::SUCCESS) {
            if (response_ata == disk_ata::OUT_OF_BOUNDS) {
                fault(-403,"Out of bounds.","ATA Hard Drive");
            } else{
                fault(-403,nullptr,"ATA Hard Drive");
            }
        }
        uint8_t* buffer = disk_ata::get_buffer();
        Terminal::print(parseInt(buffer[0],throwawayString,16));
        uint8_t writeBuffer[512] = {};
        writeBuffer[0] = 0x7f;
        response_ata = disk_ata::write_sector(0xF000000/512,writeBuffer,disk_ata::ATADrive::MASTER);
        if (response_ata != disk_ata::SUCCESS) {
            if (response_ata == disk_ata::OUT_OF_BOUNDS) {
                fault(-403,"Out of bounds.","ATA Hard Drive");
            } else if (response_ata == disk_ata::TIMEOUT) {
                fault(-403,"Timeout","ATA Hard Drive");
            } else {
                fault(-403,nullptr,"ATA Hard Drive");
            }
        }
        response_ata = disk_ata::read_sector(0xF000000/512,disk_ata::ATADrive::MASTER);
        if (response_ata != disk_ata::SUCCESS) {
            if (response_ata == disk_ata::OUT_OF_BOUNDS) {
                fault(-403,"Out of bounds.","ATA Hard Drive");
            } else{
                fault(-403,nullptr,"ATA Hard Drive");
            }
        }
        buffer = disk_ata::get_buffer();
        Terminal::print("\nATA write demso: ");
        Terminal::print(parseInt(buffer[0],throwawayString,16));
        Terminal::print("\n");
        if (buffer[0] == 0x7F) {
            Terminal::print("\nWRITE SUCCESS\n");
        }
        return DriverReturn::SUCCESS;
    }
}

namespace drivers {
}