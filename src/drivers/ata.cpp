#include <stdint.h>
#include <stddef.h>
#include "types.h"
#include "fail.h"
#include "utils.h"

#define ATA_PRIMARY 0x1F0
#define ATA_SECONDARY 0x170

#define ATA_REG_DATA 0x00
#define ATA_REG_ERROR 0x01
#define ATA_REG_SECTOR_COUNT 0x02
#define ATA_REG_LBA0 0x03
#define ATA_REG_LBA1 0x04
#define ATA_REG_LBA2 0x05
#define ATA_REG_DRIVE 0x06
#define ATA_REG_STATUS 0x07
#define ATA_REG_COMMAND 0x07
#define ATA_CMD_READ 0x20
#define ATA_CMD_WRITE 0x30
#define ATA_CMD_CACHE_FLUSH 0xE7
#define ATA_CMD_IDENT 0xEC

#define ATA_DRV_MASTER 0xA0
#define ATA_DRV_SLAVE 0xB0

#define ATA_MODE_LBA 0b01000000

#define ATA_FLAG_BSY 0x80
#define ATA_FLAG_DF 0x20
#define ATA_FLAG_DRQ 0x08
#define ATA_FLAG_ERR 0x01

/*

Copyright (C) 2026  XenithMusic (on github)

The Paranoia kernel is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Paranoia is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Paranoia. If not, see <https://www.gnu.org/licenses/>.

*/

bool driverEnabled = false;

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
    ATAState wait_bsy() {
        size_t timeout = 0x100000;
        uint8_t status = inb(ATA_PRIMARY + ATA_REG_STATUS);
        while (status & ATA_FLAG_BSY) {
            if (status & ATA_FLAG_ERR or status & ATA_FLAG_DF) {
                return FAILURE;
            }
            timeout--;
            status = inb(ATA_PRIMARY + ATA_REG_STATUS);
        }
        if (timeout == 0) {
            return TIMEOUT;
        }
        return SUCCESS;
    }
    ATAState wait_drq() {
        size_t timeout = 0x100000;
        uint8_t status = inb(ATA_PRIMARY + ATA_REG_STATUS);
        while (!(status & ATA_FLAG_DRQ) or (status & ATA_FLAG_BSY)) {
            if (status & ATA_FLAG_ERR or status & ATA_FLAG_DF) {
                return FAILURE;
            }
            timeout--;
            status = inb(ATA_PRIMARY + ATA_REG_STATUS);
        }
        if (timeout == 0) {
            return TIMEOUT;
        }
        return SUCCESS;
    }
    uint8_t buffer[512];
    uint8_t get_status() {
        return inb(ATA_PRIMARY+ATA_REG_STATUS);
    }
    ATAState read_data() {
        uint16_t word;
        for (size_t i = 0; i < 256; i++) {
            word = inw(ATA_PRIMARY+ATA_REG_DATA);
            buffer[i*2] = word&0xFF;
            buffer[i*2+1] = (word >> 8) & 0xFF;
        }
        return SUCCESS;
    }
    void select_drive(uint8_t drive) {
        outb(ATA_PRIMARY+ATA_REG_DRIVE,drive|ATA_MODE_LBA);
    }
    uint8_t* get_buffer() {
        return buffer;
    }
    ATAState ident(ATADrive drive) {
        select_drive(drive);

        // clear stuff
        outb(ATA_PRIMARY+ATA_REG_SECTOR_COUNT,0);
        outb(ATA_PRIMARY+ATA_REG_LBA0,0);
        outb(ATA_PRIMARY+ATA_REG_LBA1,0);
        outb(ATA_PRIMARY+ATA_REG_LBA2,0);

        // send ident
        outb(ATA_PRIMARY+ATA_REG_COMMAND,ATA_CMD_IDENT);

        uint8_t status = get_status();
        if (status == 0x00) {
            return NO_DRIVE;
        }
        uint8_t BSY = 1;
        while (BSY != 0) {
            status = get_status();
            BSY = status&0x80;
        }
        uint8_t LBA1 = inb(ATA_PRIMARY+ATA_REG_LBA1);
        uint8_t LBA2 = inb(ATA_PRIMARY+ATA_REG_LBA2);
        if (LBA1 != 0 or LBA2 != 0) {
            return BAD_DRIVE;
        }
        uint8_t DRQ = 0;
        uint8_t ERR = 0;
        while (DRQ == 0 and ERR == 0) {
            status = get_status();
            DRQ = status&0x8;
            ERR = status&0x1;
        }
        if (ERR != 0) {
            return FAILURE;
        }
        read_data();
        return SUCCESS;
    }
    ATAState read_sector(size_t sector, ATADrive drive) {
        size_t LBA = sector;
        select_drive(drive | ((LBA>>24)&0b1111));
        outb(ATA_PRIMARY+ATA_REG_SECTOR_COUNT,1);
        outb(ATA_PRIMARY+ATA_REG_LBA0,LBA&0xFF);
        outb(ATA_PRIMARY+ATA_REG_LBA1,(LBA>>8)&0xFF);
        outb(ATA_PRIMARY+ATA_REG_LBA2,(LBA>>16)&0xFF);

        outb(ATA_PRIMARY+ATA_REG_COMMAND,ATA_CMD_READ);
        uint8_t status = inb(ATA_PRIMARY + ATA_REG_STATUS);
        while (status & 0x80) {
            status = inb(ATA_PRIMARY + ATA_REG_STATUS);
        }
        if (status & 0x01) {
            return FAILURE;
        }
        while (!(status & 0x08)) {
            status = inb(ATA_PRIMARY + ATA_REG_STATUS);
        }
        read_data();
        return SUCCESS;
    }
    ATAState write_sector(size_t sector,uint8_t* buffer,ATADrive drive) {
        size_t LBA = sector;
        select_drive(drive | ((LBA>>24)&0b1111));
        ATAState response = wait_bsy();
        if (response != SUCCESS) return response;
        outb(ATA_PRIMARY+ATA_REG_SECTOR_COUNT,1);
        outb(ATA_PRIMARY+ATA_REG_LBA0,LBA&0xFF);
        outb(ATA_PRIMARY+ATA_REG_LBA1,(LBA>>8)&0xFF);
        outb(ATA_PRIMARY+ATA_REG_LBA2,(LBA>>16)&0xFF);

        outb(ATA_PRIMARY+ATA_REG_COMMAND,ATA_CMD_WRITE);
        response = wait_drq();
        if (response != SUCCESS) return response;
        for (size_t i=0; i < 256; i++) {
            // uint16_t word = buffer[i*2] | (buffer[(i*2)+1] << 8);
            outsw(ATA_PRIMARY+ATA_REG_DATA,buffer,256); // XXX: this is really slow on QEMU. check on real hardware to make sure it's not horribly slow.
        }
        // outb(ATA_PRIMARY+ATA_REG_COMMAND,ATA_CMD_CACHE_FLUSH);
        response = wait_bsy();
        if (response != SUCCESS) return response;
        return SUCCESS;
    }
    ATAState init() {
        // shortcut for early detection of a driveless state.
        uint8_t floatCheck = inb(ATA_PRIMARY+ATA_REG_STATUS);
        if (floatCheck == 0xFF) {
            return NO_DRIVE;
        }
        ATAState response = ident(MASTER);
        if (response != SUCCESS) {
            return response;
        }
        driverEnabled = true;
        return SUCCESS;
    }
}