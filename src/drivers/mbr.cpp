#include "types.h"
#include "genericdisk.h"

#define SECTOR_SIZE 512

namespace MBR {
    struct PartitionTableEntry {
        ubyte1_t attributes;
        ubyte1_t unused1[3];
        ubyte1_t type;
        ubyte1_t unused2[3];
        ubyte4_t sectorStart;
        ubyte4_t sectorCount;
    } __attribute__((packed));
    struct MBRStruct {
        ubyte1_t bootstrap[440];
        ubyte4_t signature;
        // if 5A5A, readonly. otherwise, this should be 0000
        ubyte2_t mode;
        PartitionTableEntry partitions[4];
        // this should be 0x55, 0xAA
        ubyte2_t magic;
    } __attribute__((packed));
    size_t getPartitionOffset(uint16_t driveIndex, uint16_t partitionIndex) {
        if (partitionIndex > 3) return -1;
        MBRStruct mbr = {};
        disk::get_bytes_raw(driveIndex,0,512,&mbr);
        return mbr.partitions[partitionIndex].sectorStart*SECTOR_SIZE;
    }
}