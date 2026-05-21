#include "types.h"
#include "memory.h"
#include "drivers/genericdisk.h"

#define SUPPORTED_REQUIRED_FEATURES 0x00
#define SUPPORTED_WRITEABLE_FEATURES 0x00

namespace ext2 {
    bool consistent = true;
    void forAllSuperblocks(disk::Filesystem* fs, void* arg, void (*fn)(disk::Filesystem* fs, void* arg, size_t byteOffset)) {
        size_t sparseLocations[] = {
            1,
            3,9,27,81,243,729,2187,6561,19683,59049,177147,
            5,25,125,625,3125,15625,78125,390625,
            7,49,343,2401,16807,117649
        };
        fn(fs,arg,1024);
        size_t group_blocks = fs->fs.ext2->superblock->group_blocks;
        size_t total_blocks = fs->fs.ext2->superblock->total_blocks;
        size_t block_groups = (total_blocks+group_blocks-1)/group_blocks;
        size_t block_size = 1024<<fs->fs.ext2->superblock->block_size;
        for (size_t i=1;i<block_groups;i++) {
            if (fs->fs.ext2->superblock->writeable_features & 0x0001) {
                bool present = false;
                for (size_t loc : sparseLocations) {
                    if (loc == i) {
                        present = true;
                    }
                }
                if (!present) continue;
            }
            fn(fs,arg,(i*group_blocks + fs->fs.ext2->superblock->superblock_no)*block_size);
        }
    }
    void writeSuperblock(disk::Filesystem* fs,size_t address) {
        if (!fs->writable) return;
        Superblock superblock;
        disk::get_bytes(fs->index,fs->partition,address,sizeof(Superblock),&superblock);
        if (superblock.signature != 0xEF53) return;
        disk::write_bytes(fs->index,fs->partition,address,sizeof(Superblock),(void*)fs->fs.ext2->superblock);
    }
    void writeSuperblock(disk::Filesystem* fs,void* unused,size_t address) {
        writeSuperblock(fs,address);
    }
    void writeSuperblocks(disk::Filesystem* fs) {
        forAllSuperblocks(fs,nullptr,writeSuperblock);
    }
    uint32_t getSuperblockChecksum(disk::Filesystem* fs, size_t bytes) {
        uint32_t checksum = 0;
        Superblock superblock;
        disk::get_bytes(fs->index,fs->partition,bytes,sizeof(Superblock),&superblock);
        if (superblock.signature != 0xEF53) return 0;
        checksum = superblock.super_blocks + superblock.block_size + superblock.frag_size + superblock.group_blocks;
        checksum += superblock.signature + superblock.err_handler;
        checksum += superblock.version_minor + superblock.version_major;
        return checksum;
    }
    void checkSuperblockChecksum(disk::Filesystem* fs, void* arg, size_t bytes) {
        uint32_t primaryChecksum = *(uint32_t*)arg;
        uint32_t thisChecksum = getSuperblockChecksum(fs,bytes);
        if (thisChecksum == 0) return;
        if (primaryChecksum != thisChecksum) {
            consistent = false;
        }
    }
    bool checkSuperblockConsistency(disk::Filesystem* fs) {
        uint32_t targetChecksum = getSuperblockChecksum(fs,1024);
        consistent = true;
        forAllSuperblocks(fs,&targetChecksum,checkSuperblockChecksum);
        if (consistent) {
            fs->fs.ext2->superblock->mounts_since_fsck = 0;
            writeSuperblocks(fs);
        }
        return consistent;
    }
    void setFilesystemState(disk::Filesystem* fs, ext2::FilesystemState state) {
        fs->fs.ext2->superblock->fs_state = state;
        writeSuperblocks(fs);
    }
    void unmount(disk::Filesystem* fs) {
        Allocator::free((void*)fs->fs.ext2->superblock);
        Allocator::free((void*)fs->fs.ext2->bgdt);
        Allocator::free((void*)fs->fs.generic);
        Allocator::free((void*)fs);
    }
    void read_block(disk::Filesystem* fs,size_t blockIndex,void* buffer) {
        if (blockIndex == 0) return;
        size_t blockSize = 1024<<fs->fs.ext2->superblock->block_size;
        size_t blockStart = blockIndex*blockSize;
        // Terminal::print("blockStart:");
        //     Terminal::print(parseU32(blockStart,10));
        //     Terminal::print("\nblockSize:");
        //     Terminal::print(parseU32(blockSize,10));
        //     Terminal::print("\n");
        disk::get_bytes(fs->index,fs->partition,blockStart,blockSize,buffer);
    }
    disk::Filesystem* mount(disk::DriveType driveType, uint16_t driveIndex, uint16_t partitionIndex) {
        disk::Filesystem* constructed = (disk::Filesystem*)Allocator::kalloc(sizeof(disk::Filesystem));
        constructed->type = driveType;
        constructed->index = driveIndex;
        constructed->partition = partitionIndex;
        constructed->fs.generic = Allocator::kalloc(sizeof(disk::FSSpecific));
        constructed->fs.ext2->superblock = (ext2::Superblock*)Allocator::kalloc(sizeof(ext2::Superblock));
        constructed->writable = true;
        disk::get_bytes(driveIndex,partitionIndex,1024,sizeof(Superblock),(void*)constructed->fs.ext2->superblock);
        if (constructed->fs.ext2->superblock->signature != 0xEF53) {
            if (constructed->fs.ext2->superblock->err_handler == ext2::ErrorHandler::FAULT) {
                fault(-800,"Ext2 signature is incorrect.");
            }
            unmount(constructed);
            return nullptr;
        }
        if (constructed->fs.ext2->superblock->required_features&(~SUPPORTED_REQUIRED_FEATURES) != 0) {
            return nullptr;
        }
        if (constructed->fs.ext2->superblock->writeable_features&(~SUPPORTED_WRITEABLE_FEATURES) != 0) {
            constructed->writable = false;
        } else {
            // TODO: update mount date, write date, mount count.
        }
        if (constructed->fs.ext2->superblock->fs_state == ext2::FilesystemState::ERRORS) {
            Terminal::print("[WARN] ext2 filesystem contains errors; mounted as read-only.\n");
            constructed->writable = false;
        }
        if (constructed->fs.ext2->superblock->mounts_since_fsck >= constructed->fs.ext2->superblock->mounts_to_fsck) {
            Terminal::print("       Running scheduled consistency check...\n");
            if (checkSuperblockConsistency(constructed) == false) {
                Terminal::print("[WARN] ext2 checksum inconsistency; mounted as read-only.\n");
                constructed->writable = false;
            } else {
                Terminal::print("[INFO] ext2 checksum consistent.\n");
            }
        }
        // referencing eduOS driver;
        // https://github.com/szhou42/osdev/blob/master/src/kernel/filesystem/ext2.c#L877
        // code was not copied directly; just used for reference.
        size_t blockSize = 1024<<constructed->fs.ext2->superblock->block_size;
        size_t groupBlocks = constructed->fs.ext2->superblock->group_blocks;
        size_t totalBlocks = constructed->fs.ext2->superblock->total_blocks;
        size_t totalGroups = (totalBlocks+groupBlocks-1)/groupBlocks;
        
        size_t bgdBytes = totalGroups*sizeof(BlockGroupDescriptor);
        size_t bgdBlocks = (bgdBytes+blockSize-1) / blockSize;
        bgdBytes = bgdBlocks*blockSize;

        size_t bgdBlock = (1024/blockSize)+1; // block after the superblock
        size_t bgdAddress = bgdBlock*blockSize;

        constructed->fs.ext2->bgdt = (BlockGroupDescriptor*)Allocator::kalloc(bgdBytes);
        // disk::get_bytes(constructed->index,constructed->partition,bgdAddress,bgdBytes,constructed->fs.ext2->bgdt);

        Terminal::print("debug mount ");
            Terminal::print(parseInt(blockSize,10));
            Terminal::print(" ");
            Terminal::print(parseInt(groupBlocks,10));
            Terminal::print(" ");
            Terminal::print(parseInt(totalBlocks,10));
            Terminal::print(" ");
            Terminal::print(parseInt(totalGroups,10));
            Terminal::print(" ");
            Terminal::print(parseInt(bgdBytes,10));
            Terminal::print(" ");
            Terminal::print(parseInt(bgdBlocks,10));
            Terminal::print(" ");
            Terminal::print(parseInt(bgdBlock,10));
            Terminal::print(" ");
            Terminal::print(parseInt((size_t)(constructed->fs.ext2->bgdt),10));
            Terminal::print(" ");

        disk::get_bytes(constructed->index,constructed->partition,bgdAddress,bgdBytes,constructed->fs.ext2->bgdt);
        if (not Allocator::is_allocated(constructed->fs.ext2->bgdt + 62)) {
            Terminal::print("\nIT'S NOT ALLOCATED FOR SOME GOD DAMN REASON WTF BRO\n");
        }
            Terminal::print(parseInt((size_t)(constructed->fs.ext2->bgdt + 62),10));
            Terminal::print(" ");
            Terminal::print(parseInt(constructed->fs.ext2->bgdt[62].inode_table_addr,10));
            Terminal::print("\n");

        // int i=0;
        // for (size_t block=0;block<=bgdBlocks;block++) {
        //     ext2::read_block(constructed,block+bgdBlock,constructed->fs.ext2->bgdt+(block*blockSize));
        //     i += 1;
        //     if (i >= 6) while(1){};
        // }

        // size_t bgdtBlock = (1024/blockSize) + 1;
        // size_t bgdtAddress = bgdtBlock*blockSize;
        
        // size_t totalBlocks = constructed->fs.ext2->superblock->total_blocks;
        // size_t totalInodes = constructed->fs.ext2->superblock->total_inodes;
        // size_t groupBlocks = constructed->fs.ext2->superblock->group_blocks;
        // size_t groupInodes = constructed->fs.ext2->superblock->group_inodes;
        // size_t bgdtCount = (totalBlocks+groupBlocks-1)/groupBlocks;
        // size_t bgdtCountViaInodes = (totalInodes+groupInodes-1)/groupInodes;
        // if (bgdtCount != bgdtCountViaInodes) {
        //     Terminal::print("[WARN] ext2 BGDT count inconsistent; not all files may be accessible.\n");
        //     // unmount(constructed);
        //     // return nullptr;
        // }
        // size_t bgdtSize = bgdtCount * sizeof(BlockGroupDescriptor);
        // Terminal::print("calculated using blocks: ");
        //     Terminal::print(parseInt(bgdtCount,10));
        //     Terminal::print("\ncalculated using inodes: ");
        //     Terminal::print(parseInt(bgdtCountViaInodes,10));
        //     Terminal::print("\n");
        // constructed->fs.ext2->bgdt = (BlockGroupDescriptor*)Allocator::kalloc(bgdtSize);
        // disk::get_bytes(
        //     constructed->index,
        //     constructed->partition,
        //     bgdtAddress,
        //     bgdtSize,
        //     constructed->fs.ext2->bgdt);
        // Terminal::print("sizeof(BGDT)=");
        //     Terminal::print(parseInt(sizeof(BlockGroupDescriptor),10));

        //     Terminal::print("\nfirst inode table=");
        //     Terminal::print(parseU32(
        //         constructed->fs.ext2->bgdt[0].inode_table_addr,
        //     16));

        //     Terminal::print("\n62nd inode table=");
        //     Terminal::print(parseU32(
        //         constructed->fs.ext2->bgdt[61].inode_table_addr,
        //     16));

        //     Terminal::print("\nBGDT location=");
        //     Terminal::print(parseU32(
        //         bgdtAddress,
        //     16));

        //     Terminal::print("\nBGDT size=");
        //     Terminal::print(parseU32(
        //         bgdtSize,
        //     16));

        //     Terminal::print("\nBGDT pointer=");
        //     Terminal::print(parseU32(
        //         (size_t)constructed->fs.ext2->bgdt,
        //     16));

        //     Terminal::print("\n");
        constructed->fs.ext2->superblock->mounts_since_fsck++;
        // writeSuperblocks(constructed);
        return constructed;
    }
    Inode* open_inode(disk::Filesystem* fs, size_t address) {
        address--;
        size_t blockSize = 1024<<fs->fs.ext2->superblock->block_size;
        size_t groupInodes = fs->fs.ext2->superblock->group_inodes;
        size_t groupIndex = address/groupInodes;
        size_t inode_table_block = fs->fs.ext2->bgdt[groupIndex].inode_table_addr;
        if (inode_table_block == 0) {
            Terminal::print("inodeTable is 0\n");
            Terminal::print("it SHOULD be located at ");
                Terminal::print(parseInt((size_t)(fs->fs.ext2->bgdt + (groupIndex)),10));
                Terminal::print("\n");
            Terminal::print("groupIndex is ");
                Terminal::print(parseInt(groupIndex,10));
                Terminal::print("\n");
            Terminal::print("manually indexing the bgdt by 62 yields ");
                Terminal::print(parseInt((size_t)(fs->fs.ext2->bgdt + (62)),10));
                Terminal::print("\n");
            while(1){}
        }
        size_t inodeGroupIndex = address%groupInodes;
        size_t block_offset = (inodeGroupIndex) * fs->fs.ext2->superblock->inode_size / blockSize;
        size_t offset_in_block = (inodeGroupIndex) - block_offset * (blockSize/fs->fs.ext2->superblock->inode_size);
        char* temp = (char*)Allocator::kalloc(blockSize);
        read_block(fs,inode_table_block + block_offset, temp);
        Inode* inode = (Inode*)Allocator::kalloc(fs->fs.ext2->superblock->inode_size);
        kmemcpy(inode,temp + offset_in_block * fs->fs.ext2->superblock->inode_size,fs->fs.ext2->superblock->inode_size);
        Allocator::free(temp);
        return inode;

        // Terminal::print("opening inode...\n");
        // address--;
        // size_t blockSize = 1024<<fs->fs.ext2->superblock->block_size;
        // size_t groupInodes = fs->fs.ext2->superblock->group_inodes;
        // size_t groupIndex = address/groupInodes;
        // size_t inodeGroupIndex = address%groupInodes;
        // size_t inodeTableDiskAddrBlocks = fs->fs.ext2->bgdt[groupIndex].inode_table_addr;
        // if (inodeTableDiskAddrBlocks == 0) {
        //     Terminal::print("inodeTable is 0\n");
        //     return nullptr;
        // }
        // size_t inodeSizeBytes = 128;
        // if (fs->fs.ext2->superblock->version_major >= 1) inodeSizeBytes = fs->fs.ext2->superblock->inode_size;
        // Terminal::print("- inodeSizeBytes is ");
        //     Terminal::print(parseInt(inodeSizeBytes,10));
        //     Terminal::print("\n");
        
        // size_t inodeTableDiskAddrBytes = inodeTableDiskAddrBlocks*blockSize;
        // size_t inodeDiskAddrBytes = inodeTableDiskAddrBytes + (inodeGroupIndex*inodeSizeBytes);
        // Inode* inode = (Inode*)Allocator::kalloc(inodeSizeBytes);
        // disk::get_bytes(fs->index,fs->partition,inodeDiskAddrBytes,inodeSizeBytes,inode);
        // if (inode->types_permissions&0xF000 == 0) {
        //     Terminal::print("inode types == 0???\n");
        // }
        // return inode;
    }
    void read_inode_block(disk::Filesystem* fs,Inode* inode, size_t block, void* buffer) {
        size_t blockSize = 1024<<fs->fs.ext2->superblock->block_size;
        if (block < 12) {
            Terminal::print("direct\n");
            read_block(fs,inode->direct_pointers[block],buffer);
            return;
        }
        block -= 12;
        size_t singlyIndirectCount = blockSize/sizeof(uint32_t);
        uint32_t* singleIndirectBuffer = (uint32_t*)Allocator::kalloc(singlyIndirectCount*4);
        if (block < singlyIndirectCount) {
            read_block(fs,inode->pointer_single_indirect,singleIndirectBuffer);
            read_block(fs,singleIndirectBuffer[block],buffer);
            Allocator::free(singleIndirectBuffer);
            return;
        }
        block -= singlyIndirectCount;
        size_t doublyIndirectCount = singlyIndirectCount*singlyIndirectCount;
        uint32_t* doubleIndirectBuffer = (uint32_t*)Allocator::kalloc(singlyIndirectCount*4);
        if (block < doublyIndirectCount) {
            size_t doubleIndirectIndex = block/singlyIndirectCount;
            size_t singleIndirectIndex = block%singlyIndirectCount;
            read_block(fs,inode->pointer_double_indirect,doubleIndirectBuffer);
            read_block(fs,doubleIndirectBuffer[doubleIndirectIndex],singleIndirectBuffer);
            read_block(fs,singleIndirectBuffer[singleIndirectIndex],buffer);
            Allocator::free(doubleIndirectBuffer);
            Allocator::free(singleIndirectBuffer);
            return;
        }
        block -= doublyIndirectCount;
        size_t triplyIndirectCount = doublyIndirectCount*singlyIndirectCount;
        uint32_t* tripleIndirectBuffer = (uint32_t*)Allocator::kalloc(singlyIndirectCount*4);
        if (block < triplyIndirectCount) {
            size_t tripleIndirectIndex = block/doublyIndirectCount;
            size_t doubleIndirectIndex = (block%doublyIndirectCount)/singlyIndirectCount;
            size_t singleIndirectIndex = block%singlyIndirectCount;
            read_block(fs,inode->pointer_triple_indirect,tripleIndirectBuffer);
            read_block(fs,tripleIndirectBuffer[tripleIndirectIndex],doubleIndirectBuffer);
            read_block(fs,doubleIndirectBuffer[doubleIndirectIndex],singleIndirectBuffer);
            read_block(fs,singleIndirectBuffer[singleIndirectIndex],buffer);
        }
        Allocator::free(tripleIndirectBuffer);
        Allocator::free(doubleIndirectBuffer);
        Allocator::free(singleIndirectBuffer);
        return;
    }
    void read_inode(disk::Filesystem* fs,Inode* inode, size_t startAddress, size_t bytes, void* buffer) {
        size_t blockSize = 1024<<fs->fs.ext2->superblock->block_size;
        size_t startBlockIndex = startAddress/blockSize;
        size_t startBlockOffset = startAddress%blockSize;
        size_t endAddress = startAddress+bytes;
        size_t endBlockIndex = (endAddress)/blockSize;
        size_t endBlockOffset = blockSize-(endAddress%blockSize);
        uint8_t* tempBuffer = (uint8_t*)Allocator::kalloc(blockSize);
        uint8_t* userBuffer = (uint8_t*)buffer;
        size_t accumulated = 0;
        for (size_t block=startBlockIndex;block <= endBlockIndex;block++) {
            Terminal::print("(read_inode) block: ");
                Terminal::print(parseU32(block,10));
                Terminal::print("\n");
            read_inode_block(fs,inode,block,tempBuffer);
            size_t srcOffset = 0;
            size_t endOffset = 0;
            if (block == startBlockIndex) srcOffset = startBlockOffset;
            if (block == endBlockIndex) endOffset = endBlockOffset;
            kmemcpy(userBuffer+accumulated,tempBuffer+srcOffset,blockSize-srcOffset-endOffset);
            accumulated += blockSize-srcOffset-endOffset;
        }
        Allocator::free(tempBuffer);
    }
    Pair<bool,DirectoryMetadata> get_child(disk::Filesystem* fs, Inode* inode, char* child_name) {
        size_t blockSize = 1024<<fs->fs.ext2->superblock->block_size;
        uint8_t* buffer = (uint8_t*)Allocator::kalloc(blockSize);
        uint64_t maximumBlock = (inode->lower_size+blockSize-1)/blockSize;
        DirectoryEntry* child = nullptr;
        size_t nameLen = strlen(child_name);
        if (fs->fs.ext2->superblock->version_major >= 1) {
            maximumBlock |= inode->upper_size<<32;
        }
        DirectoryEntry* entry = nullptr;
        for (size_t block = 0;block <= maximumBlock && child == nullptr;block++) {
            read_inode_block(fs, inode, block, buffer);
            size_t position = 0;
            while (position < blockSize) {
                entry = (DirectoryEntry*)(buffer+position);
                if ( // validity check
                    entry->metadata.name_len == nameLen and 
                    entry->metadata.inode != 0) 
                    {
                    int comparison = kmemcmp(entry->name,child_name,entry->metadata.name_len);
                    Terminal::print("... ");
                        Terminal::print(entry->name);
                        Terminal::print(" at length ");
                        Terminal::print(parseInt(entry->metadata.name_len,10));
                        Terminal::print(" compared to ");
                        Terminal::print(parseInt(comparison,10));
                        Terminal::print("\n");
                    if (comparison == 0) {
                        child = entry;
                        goto child_found;
                    }
                }
                position += entry->metadata.total_size;
                while (position < blockSize && buffer[position] == 0) {
                    position++; // NOTE: could be 4x faster; this does not skip unaligned positions
                }
            }
        }
        child_found:
        Pair<bool,DirectoryMetadata> returnValue = {false,NULL};
        if (child != nullptr) {
            returnValue.first = true;
            returnValue.second = *((DirectoryMetadata*)child);
        }
        Allocator::free(buffer);
        return returnValue;
    }
    void close_inode(Inode* inode) {
        Allocator::free(inode);
    }
}