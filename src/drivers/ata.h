#include <stdint.h>
#include <stddef.h>

#pragma once
/*

Copyright (C) 2026  XenithMusic (on github)

The Paranoia kernel is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Paranoia is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Paranoia. If not, see <https://www.gnu.org/licenses/>.

*/

namespace disk_ata {
    enum ATAState {
        SUCCESS = 0,
        FAILURE,
        OUT_OF_BOUNDS,
        NO_DRIVE,
        BAD_DRIVE,
        TIMEOUT
    };
    
    enum ATADrive {
        MASTER = 0xA0,
        SLAVE = 0xB0
    };

    uint8_t* get_buffer();
    ATAState read_sector(size_t sector, ATADrive drive);
    ATAState write_sector(size_t sector,uint8_t* buffer, ATADrive drive);
    ATAState init();
}