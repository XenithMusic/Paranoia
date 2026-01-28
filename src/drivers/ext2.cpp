#include "ata.h"
#include "const.h"
#include "memory.h"
#include "types.h"
#include "terminal.h"
#include "string.h"
#include "math.h"

// How large sectors returned by the Disk Driver are.
#define SECTOR_SIZE 512

// Byte offset that the ext2 filesystem starts at.
#define FS_OFF 64*MEBIBYTES

// Sector offset that the ext2 filesystem starts at.
#define FS_OFF_SECTORS FS_OFF/SECTOR_SIZE

// The largest size of a block. (in bytes)
// This is equivalent to the size of the ext2's buffer.
#define MAX_BLOCK_SIZE 4096

/*

Copyright (C) 2026  XenithMusic (on github)

The Paranoia kernel is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Paranoia is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Paranoia. If not, see <https://www.gnu.org/licenses/>.

*/

namespace ext2 {
    char throwawayString[256];
    Superblock superblock;
    BlockGroupDescriptor* bgd_table;
    bool allowWrites;
    size_t block_size;
    size_t frag_size;
    uint8_t buffer[MAX_BLOCK_SIZE]; // best to declare a fixed-size buffer because i'm in the kernel; dynamic alloc is not the best here.
    enum ExtState {
        SUCCESS,
        FAILURE,
        UNALIGNED,
        LARGE,
        BAD_FS,
        UNCLEAN,
        NULL_VALUE,
        NOT_FOUND
    };
    /**
     * This is intended for internal use. If you can pass `userBuffer` to a function instead,
     * you should.
     * 
     * Gets the buffer from the driver, and copies it to a buffer passed by the user.
     * bufferSize should be equivalent to the blockSize of the read.
     * 
     * Returns:
     * - SUCCESS
     * - LARGE (blockSize is larger than the maximum allowed block size.)
     */
    ExtState get_buffer(size_t bufferSize, void* userBuffer) {
        if (bufferSize > MAX_BLOCK_SIZE) {
            return LARGE;
        }
        kmemcpy(userBuffer,buffer,bufferSize);
        return SUCCESS;
    }
    ExtState get_superblock(Superblock* userBuffer) {
        kmemcpy(userBuffer,&superblock,sizeof(Superblock));
        return SUCCESS;
    }
    /**
     * This overload does not output a buffer, and instead it must be retrieved with ext2::get_buffer.
     * Pass `userBuffer` to copy the buffer to your own buffer.
     * 
     * Reads one block from the disk.
     * The first block (0) will point to the ext2 file system, not the kernel.
     * blockSize is passed in bytes.
     * 
     * If void* userBuffer is passed, the resulting buffer is `kmemcpy`ied to the passed pointer.
     * 
     * Returns:
     * - SUCCESS
     * - UNALIGNED (blockSize is not aligned to the sector size.)
     * - LARGE (blockSize is larger than the maximum allowed block size.)
     */
    ExtState read_block(size_t blockNo,size_t blockSize) {
        if (blockSize % SECTOR_SIZE != 0) {
            return UNALIGNED;
        } else if (blockSize > MAX_BLOCK_SIZE) {
            return LARGE;
        }
        size_t sectorsPerBlock = blockSize/SECTOR_SIZE;
        size_t startOffset = (blockNo)*sectorsPerBlock+FS_OFF_SECTORS;
        for (size_t i=0;i < sectorsPerBlock;i++) {
            disk_ata::read_sector(startOffset+i,disk_ata::ATADrive::MASTER);
            kmemcpy(buffer+(i*SECTOR_SIZE),disk_ata::get_buffer(),512);
        }
        return SUCCESS;
    }
    /**
     * Reads one block from the disk.
     * The first block (0) will point to the ext2 file system, not the kernel.
     * blockSize is passed in bytes.
     * 
     * The resulting buffer is `kmemcpy`ied to the passed `userBuffer`.
     * 
     * Returns:
     * - SUCCESS
     * - UNALIGNED (blockSize is not aligned to the sector size.)
     * - LARGE (blockSize is larger than the maximum allowed block.)
     */
    ExtState read_block(size_t blockNo, size_t blockSize, void* userBuffer) {
        ExtState result = read_block(blockNo, blockSize);
        if (result != SUCCESS) return result;
        return get_buffer(blockSize,userBuffer);
    }
    /**
     * Reads the superblock from the disk.
     * There is no version of this function that will not write a Superblock buffer to a user-specified pointer.
     * However, if you don't care, you can pass a nullptr, and it will be ignored.
     * 
     * Returns:
     * - SUCCESS
     * - UNALIGNED (blockSize is not aligned to the sector size.)
     * - LARGE (blockSize is larger than the maximum allowed block size.)
     * - BAD_FS (resulting superblock does not have a valid ext2 file signature.)
     */
    ExtState read_superblock(Superblock* userBuffer) {
        ExtState response = read_block(1,1024); // NOTE: assume the block size is 1024 to start reading from 1024 bytes. this is correct no matter the block size.
        if (response != SUCCESS) {
            return response;
        }
        get_buffer(sizeof(Superblock),&superblock);
        if (superblock.signature != 0xEF53) {
            return BAD_FS;
        }
        if (userBuffer != nullptr) get_superblock(userBuffer);
        return SUCCESS;
    }
    /**
     * Gets the size of the Block Group Descriptor Table in bytes.
     */
    size_t get_bgdt_size(Superblock* superblock) {
        size_t total = superblock->total_blocks;
        size_t group = superblock->group_blocks;
        size_t num_groups = (total+group-1)/group;
        return sizeof(BlockGroupDescriptor)*num_groups;
    }
    /**
     * Reads the Block Group Descriptor Table from the disk.
     * There is no version of this function that will not write a buffer to a user-specified pointer.
     * 
     * Returns:
     * - SUCCESS
     * - NULL_VALUE (not enough information is known; the table must be assumed to be null.)
     * - UNALIGNED (blockSize is not aligned to the sector size.)
     * - LARGE (blockSize is larger than the maximum allowed block.)
     */
    ExtState read_bgd_table(BlockGroupDescriptor* userBuffer,Superblock* superblock) {
        uint32_t bgdt_block = 2;
        if (superblock->block_size > 0) {
            bgdt_block = 1;
        }
        size_t bgdt_size = get_bgdt_size(superblock);
        if (bgdt_size == 0) {
            return NULL_VALUE;
        }
        size_t real_block_size = 1024 << superblock->block_size;
        for (size_t offset = 0;offset < bgdt_size;offset += real_block_size) {
            ExtState response = read_block(bgdt_block+(offset/real_block_size),real_block_size);
            if (response != SUCCESS) return response;
            kmemcpy((uint8_t*)userBuffer+offset,buffer,min(real_block_size,bgdt_size-offset));
        }
        return SUCCESS;
    }
    /**
     * Returns the Inode Table.
     * 
     * Address is in blocks.
     * 
     * Returns:
     * - Inode* always.
     */
    Inode* get_inode_table(size_t address,Superblock* superblock,BlockGroupDescriptor* bgd_table) {
        // FATAL: This is completely non-functional, and yields garbage.
        size_t group = (address-1)/superblock->group_inodes;
        size_t table_addr = bgd_table[group].inode_table_addr;
        return (Inode*)table_addr;
    }
    /**
     * Returns an Inode.
     * 
     * Warning:
     * - Make sure the type (inode->types_permissions&0xF000) is not 0.
     * - This will not fail on an invalid inode.
     * 
     * Returns:
     * - Inode* always.
     */
    Inode* get_inode(size_t address,Superblock* superblock,BlockGroupDescriptor* bgd_table) {
        Inode* inodetable = get_inode_table(address,superblock,bgd_table);
        size_t index = (address-1)%superblock->group_inodes;
        Inode* inode = inodetable+index;
        return inode;
    }
    /**
     * Reads a block from an Inode.
     * 
     * BUG: This does not work for the 13th block or above, because indirect blocks have not been handled.
     * 
     * Returns:
     * - SUCCESS
     * - UNALIGNED (blockSize is not aligned to the sector size.)
     * - LARGE (blockSize is larger than the maximum allowed block size.)
     */
    ExtState read_inode_data(size_t blockIndex, Inode* inode, Superblock* superblock) {
        if (blockIndex > 11) { // BUG: Does not work with indirect blocks.
            fault(2,"Indirect blocks are not supported yet. Pain.","ext2 Filesystem (internal)");
        }
        return read_block(inode->direct_pointers[blockIndex],superblock->block_size);
    }

    /**
     * Returns a directory entry notating the child of a directory.
     * 
     * Returns:
     * - Pair<ExtState,DirectoryEntry>
     * 
     * ExtState:
     * - SUCCESS
     * - UNALIGNED (blockSize is not aligned to the sector size.)
     * - LARGE (blockSize is larger than the maximum allowed block size.)
     * 
     */
    Pair<ExtState,ubyte4_t> get_child(char name[], size_t parentAddress,Superblock* superblock, BlockGroupDescriptor* bgd_table) {
        // BUG: THIS WILL NOT WORK WITH DIRECTORIES THAT NEED MORE THAN 1 BLOCKS. THIS COULD BREAK WITH AS LOW AS 3 CHILDREN.
        //
        //       ...theoretically it could be 1, however names longer than 255 characters are not permitted
        Inode* parent = get_inode(parentAddress,superblock,bgd_table);
        for (size_t block = 0; block < 5; block++) {
            Terminal::print("ADDRESS: ");
                Terminal::print(parseInt(parent->direct_pointers[block],throwawayString,16));
                Terminal::print("\n");
            ExtState response = read_inode_data(block,parent,superblock);
            if (response != SUCCESS) {
                return {response,0};
            }
            DirectoryEntry* directories = (DirectoryEntry*)buffer;
            size_t offset = 0;
            DirectoryEntry* currentDirectory;
            while (offset < block_size) {
                currentDirectory = (DirectoryEntry*)((ubyte_t*)directories + offset);
                if (currentDirectory->inode != 0) {
                    Terminal::print("inode name_len total_size: ");
                        Terminal::print(parseInt(currentDirectory->inode,throwawayString,16));
                        Terminal::print(" ");
                        Terminal::print(parseInt(currentDirectory->name_len,throwawayString,16));
                        Terminal::print(" ");
                        Terminal::print(parseInt(currentDirectory->total_size,throwawayString,16));
                        Terminal::print("\n");
                    if (currentDirectory->name_len == strlen(name) and
                    kmemcmp(name,currentDirectory->name,currentDirectory->name_len)) {
                        return {SUCCESS,currentDirectory->inode};
                    }
                }
                if (currentDirectory->total_size == 0) {
                    break;
                }
                offset += currentDirectory->total_size;
            }
        }
        return {NOT_FOUND,0};
    }
    /**
     * Reads the superblock from the disk.
     * There is no version of this function that will not write a Superblock buffer to a user-specified pointer.
     * However, if you don't care, you can pass a nullptr, and it will be ignored.
     * 
     * Returns:
     * - SUCCESS
     * - UNALIGNED (blockSize is not aligned to the sector size.)
     * - LARGE (blockSize is larger than the maximum allowed block size.)
     * - BAD_FS (resulting superblock does not have a valid ext2 file signature.)
     * - UNCLEAN (file system has errors.)
     */
    ExtState init() {
        ExtState successResponse = SUCCESS;
        ExtState response = read_superblock(&superblock);
        if (response != SUCCESS) {
            return response;
        }
        allowWrites = true;
        if (superblock.fs_state == ERRORS) {
            successResponse = UNCLEAN;
            switch (superblock.err_handler) {
                case FAULT:
                    while (true) { fault(-800,nullptr,"ext2"); }
                    break;
                case READONLY:
                    allowWrites = false;
                    break;
                case SKIP:
                    break;
                default:
                    break;
            }
        }
        response = read_bgd_table(bgd_table,&superblock);
        if (response != SUCCESS) return response;
        block_size = 1024 << superblock.block_size;
        frag_size = 1024 << superblock.frag_size;
        return successResponse;
    }
}