#include "types.h"
#include "drivers/ata.h"
#include "memory.h"
#include "mbr.h"
#include <pit.h>

#define SECTOR_SIZE 512

namespace disk {
    // bool get_bytes_raw(uint16_t driveIndex, size_t startAddress,size_t bytes,void* destination) {
    //     size_t start_sector = startAddress/SECTOR_SIZE;
    //     size_t endAddress = startAddress + bytes - 1;
    //     size_t endSector = endAddress/SECTOR_SIZE;
    //     size_t startIndex = startAddress%SECTOR_SIZE;
    //     size_t endIndex = endAddress%SECTOR_SIZE;
    //     size_t writtenBytes = 0;
    //     for (size_t currentSector=start_sector;currentSector<=endSector;currentSector++) {
    //         if (disk_ata::read_sector(currentSector,(disk_ata::ATADrive)driveIndex) != disk_ata::ATAState::SUCCESS) {
    //             return false;
    //         }
    //         size_t indexOffset = 0;
    //         size_t copyCount = SECTOR_SIZE;
    //         if (currentSector == start_sector) indexOffset = startIndex;
    //         if (currentSector == endSector) copyCount = endIndex + 1;
    //         kmemcpy((ubyte_t*)destination+writtenBytes,disk_ata::get_buffer()+indexOffset,copyCount-indexOffset);
    //         writtenBytes += copyCount-indexOffset;
    //     }
    //     if (writtenBytes != bytes) fault(80085,"writtenBytes != bytes");
    //     return true;
    // }
    bool get_bytes_raw(uint16_t driveIndex, size_t startAddress,size_t bytes,void* destination) {
        size_t endAddress = startAddress+bytes;
        size_t firstSector = startAddress/SECTOR_SIZE;
        size_t lastSector = endAddress/SECTOR_SIZE;
        size_t firstOffset = startAddress%SECTOR_SIZE;
        size_t lastOffset = endAddress%SECTOR_SIZE;
        size_t accumulated = 0;
        for (size_t sector=firstSector;sector <= lastSector;sector++) {
            if (disk_ata::read_sector(sector,(disk_ata::ATADrive)driveIndex) != disk_ata::ATAState::SUCCESS) {
                return false;
            }
            size_t index = 0;
            size_t size = SECTOR_SIZE;
            if (sector==lastSector) {
                size = lastOffset;
            }
            if (sector==firstSector) {
                index += firstOffset;
                size -= firstOffset;
            }
            kmemcpy((ubyte_t*)destination+accumulated,disk_ata::get_buffer()+index,size);
            accumulated += size;
            // Terminal::print("acc ");
            //     Terminal::print(parseInt(size,10));
            //     Terminal::print(" index ");
            //     Terminal::print(parseInt(index,10));
            //     Terminal::print(" first ");
            //     Terminal::print(parseInt(firstSector,10));
            //     Terminal::print(" fioff ");
            //     Terminal::print(parseInt(firstOffset,10));
            //     Terminal::print(" second ");
            //     Terminal::print(parseInt(lastSector,10));
            //     Terminal::print(" secoff ");
            //     Terminal::print(parseInt(lastOffset,10));
            //     Terminal::print("\n");
        }
        if (accumulated != bytes) {
            Terminal::print("error ");
            Terminal::print(parseInt(accumulated,10));
            Terminal::print("\n");
            Terminal::print("error ");
            Terminal::print(parseInt(bytes,10));
            Terminal::print("\n");
            while(1){}
        }
        return true;
    }
    void breakpoint(){busy_sleep(10);}
    bool get_bytes(uint16_t driveIndex, uint16_t partitionIndex, size_t startAddress,size_t bytes,void* destination) {
        size_t partitionOffset = MBR::getPartitionOffset(driveIndex,partitionIndex);
        get_bytes_raw(driveIndex,startAddress+partitionOffset,bytes,destination);
        // if (startAddress > 1024) breakpoint();
        return true;
    }
    bool write_bytes_raw(uint16_t driveIndex,size_t startAddress,size_t bytes,void* source) {
        size_t startSectors = (startAddress/SECTOR_SIZE);
        size_t alignedStart = startSectors*SECTOR_SIZE;
        size_t offset = startAddress-alignedStart;
        size_t bytesSectors = (offset + bytes + SECTOR_SIZE - 1) / SECTOR_SIZE;
        size_t alignedBytes = bytesSectors*SECTOR_SIZE;
        size_t endSector = startSectors+bytesSectors;
        void* originalData = Allocator::kalloc(alignedBytes);
        get_bytes_raw(driveIndex,alignedStart,alignedBytes,originalData);
        uint8_t* newData = (uint8_t*)originalData;
        kmemcpy(newData + offset,source,bytes);
        size_t i = 0;
        for (size_t currentSector=startSectors;currentSector<endSector;currentSector++) {
            if (disk_ata::write_sector(currentSector,newData + (i*SECTOR_SIZE),(disk_ata::ATADrive)driveIndex) != disk_ata::ATAState::SUCCESS) {
                return false;
            }
            i++;
        }
        Allocator::free(originalData);
        return true;
    }
    bool write_bytes(uint16_t driveIndex, uint16_t partitionIndex, size_t startAddress,size_t bytes,void* source) {
        size_t partitionOffset = MBR::getPartitionOffset(driveIndex,partitionIndex);
        return write_bytes_raw(driveIndex,startAddress+partitionOffset,bytes,source);
    }
}