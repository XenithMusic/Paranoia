#include "terminal.h"
#include "string.h"
#include "types.h"
#include "acpi.h"
#include "fail.h"
#include "ext2.h"
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
        // Terminal::print("ATA read demo: ");
        // response_ata = disk_ata::read_sector(0xF000000/512,disk_ata::ATADrive::MASTER);
        // if (response_ata != disk_ata::SUCCESS) {
        //     if (response_ata == disk_ata::OUT_OF_BOUNDS) {
        //         fault(-403,"Out of bounds.","ATA Hard Drive");
        //     } else{
        //         fault(-403,nullptr,"ATA Hard Drive");
        //     }
        // }
        // uint8_t* buffer = disk_ata::get_buffer();
        // Terminal::print(parseInt(buffer[0],throwawayString,16));
        // uint8_t writeBuffer[512] = {};
        // writeBuffer[0] = 0x7f;
        // response_ata = disk_ata::write_sector(0xF000000/512,writeBuffer,disk_ata::ATADrive::MASTER);
        // if (response_ata != disk_ata::SUCCESS) {
        //     if (response_ata == disk_ata::OUT_OF_BOUNDS) {
        //         fault(-403,"Out of bounds.","ATA Hard Drive");
        //     } else if (response_ata == disk_ata::TIMEOUT) {
        //         fault(-403,"Timeout","ATA Hard Drive");
        //     } else {
        //         fault(-403,nullptr,"ATA Hard Drive");
        //     }
        // }
        // response_ata = disk_ata::read_sector(0xF000000/512,disk_ata::ATADrive::MASTER);
        // if (response_ata != disk_ata::SUCCESS) {
        //     if (response_ata == disk_ata::OUT_OF_BOUNDS) {
        //         fault(-403,"Out of bounds.","ATA Hard Drive");
        //     } else{
        //         fault(-403,nullptr,"ATA Hard Drive");
        //     }
        // }
        // buffer = disk_ata::get_buffer();
        // Terminal::print("\nATA write demso: ");
        // Terminal::print(parseInt(buffer[0],throwawayString,16));
        // Terminal::print("\n");
        // if (buffer[0] == 0x7F) {
        //     Terminal::print("\nWRITE SUCCESS\n");
        // }

        ext2::ExtState response_ext2 = ext2::init();
        if (response_ext2 != ext2::SUCCESS) {
            if (response_ext2 == ext2::BAD_FS) {
                fault(-403,"Bad filesystem signature.","ext2 Filesystem");
            } else if (response_ext2 == ext2::UNALIGNED) {
                fault(-403,"Block Size is unaligned.","ext2 Filesystem");
            } else if (response_ext2 == ext2::LARGE) {
                fault(-403,"Block Size is too large.","ext2 Filesystem");
            } else if (response_ext2 == ext2::UNCLEAN) {
                fault(-403,"File system has errors.","ext2 Filesystem");
            }
            else {
                fault(-403,nullptr,"ext2 Filesystem");
            }
        }
        Terminal::print("a\n");
        ext2::Superblock superblock;
        response_ext2 = ext2::get_superblock(&superblock);
        if (response_ext2 != ext2::SUCCESS) {
            fault(-403,"demo 1","ext2 Filesystem");
        }
        Terminal::print("have superblock\n");
        ext2::BlockGroupDescriptor bgd_table[superblock.total_blocks / superblock.group_blocks + 1];
        response_ext2 = ext2::read_bgd_table(bgd_table,&superblock);
        if (response_ext2 != ext2::SUCCESS) {
            fault(-403,"demo 2","ext2 Filesystem");
        }
        Terminal::print("have bgd\n");
        Pair<ext2::ExtState,ubyte4_t> child = ext2::get_child("demo",2,&superblock,bgd_table);
        if (child.first != ext2::SUCCESS) {
            if (child.first == ext2::NOT_FOUND) {
                fault(-403,"demo 3: not found","ext2 Filesystem");
            }
            fault(-403,"demo 3","ext2 Filesystem");
        }
        Terminal::print("have child\n");
        Pair<ext2::ExtState,ubyte4_t> child2 = ext2::get_child("file.txt",child.second,&superblock,bgd_table);
        while(true);
        if (child2.first != ext2::SUCCESS) {
            if (child2.first == ext2::NOT_FOUND) {
                fault(-403,"demo 4: not found","ext2 Filesystem");
            }
            fault(-403,"demo 4","ext2 Filesystem");
        }
        Terminal::print("have file\n");
        ext2::Inode* inode2 = ext2::get_inode(child2.second,&superblock,bgd_table);
        response_ext2 = ext2::read_inode_data(0,inode2,&superblock);
        if (response_ext2 != ext2::SUCCESS) {
            fault(-403,"demo 5","ext2 Filesystem");
        }
        char character = 32;
        response_ext2 = ext2::get_buffer(1,&character);
        if (response_ext2 != ext2::SUCCESS) {
            fault(-403,"demo 6","ext2 Filesystem");
        }
        Terminal::print("FIRST CHAR:");
            Terminal::print(parseInt(inode2->direct_pointers[0]>>16,throwawayString,16));
            Terminal::print(":");
            Terminal::print(parseInt(offsetof(ext2::Inode,direct_pointers),throwawayString,16));
            Terminal::print("\n");
        Terminal::print("[");
            Terminal::print(parseDouble(get_pit_seconds(),throwawayString,10));
            Terminal::print("] Initialized ext2 Filesystem Driver.");
            Terminal::print("\n");
        return DriverReturn::SUCCESS;
    }
}

namespace drivers {
}