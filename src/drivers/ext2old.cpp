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
#define MAX_INODE_INDEX 18000000000000000000ULL/MAX_BLOCK_SIZE

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
        NOT_FOUND,
        BAD_PATH
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
    ExtState get_buffer(size_t bufferSize, void* userBuffer, size_t seek) {
        if (bufferSize > MAX_BLOCK_SIZE) {
            return LARGE;
        }
        kmemcpy(userBuffer,buffer+seek,bufferSize);
        return SUCCESS;
    }
    ExtState get_buffer(size_t bufferSize, void* userBuffer) {
        return get_buffer(bufferSize,userBuffer,0);
    }
    ExtState get_superblock(Superblock* userBuffer) {
        kmemcpy(userBuffer,&superblock,sizeof(Superblock));
        return SUCCESS;
    }
    ExtState get_bgdt(BlockGroupDescriptor* userBuffer) {
        userBuffer = bgd_table;
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
    // Inode* get_inode_table(size_t address,Superblock* superblock,BlockGroupDescriptor* bgd_table) {
    //     // f: This ~~is~~ was non-functional, but has been rewritten.    //     return (Inode*)table_addr;
    // }
    /**
     * Returns the Inode Table's address for a certain BlockGroupDescriptor.
     * 
     * Address is in blocks.
     * 
     * Returns:
     * - ubyte4_t / uint32_t always.
     */
    ubyte4_t get_inode_table_address(size_t group,BlockGroupDescriptor* bgd_table) {
        ubyte4_t table_addr = bgd_table[group].inode_table_addr;
        return table_addr;
    }
    /**
     * Returns an Inode, and writes it to ubuffer.
     * 
     * Warning:
     * - Make sure the type (inode->types_permissions&0xF000) is not 0.
     * - This will not fail on an invalid inode.
     */
    void get_inode(Inode* ubuffer, size_t address,Superblock* superblock,BlockGroupDescriptor* bgd_table) {
        if (address == 0) fault(-404,"address should not be 0");
        if (address > MAX_INODE_INDEX) fault(-801,"attempted access of inode out of bounds");
        if (ubuffer == nullptr) fault(-404,"ubuffer is null");
        if (superblock == nullptr) fault(-404,"superblock is null");
        if (bgd_table == nullptr) fault(-404,"bgd_table is null");
        if (superblock->group_inodes == 0) fault(-403,"group inodes is 0");
        size_t index = address-1;
        size_t per_group = superblock->group_inodes;
        size_t group = index/per_group;
        size_t offset = index%per_group;

        size_t inode_table_base = get_inode_table_address(group,bgd_table)*block_size;

        size_t inode_size = sizeof(Inode); // this assumes that the Inode struct is sized correctly.
        if (superblock->version_major >= 1) {
            if (superblock->required_features != 0) {
                fault(-403,"Unsupported Required Features","ext2 Filesystem");
            }
            inode_size = superblock->inode_size;
            if (inode_size < 128) fault(-403,"Filesystem is corrupt; inode_size < 128>");
            if (inode_size > MAX_BLOCK_SIZE) fault(-403,"Filesystem is corrupt; inode_size > MAX_BLOCK_SIZE");
            if (inode_size%4 != 0) fault(-403,"Filesystem is corrupt; inode_size%4 != 0");
        }
        // BUG: extremely rare chance that a large inode_table_base caused by file system corruption
        //      can exceed the 64-bit integer limit despite an address smaller than MAX_INODE_INDEX.
        //      if someone somehow gets this error through fs corruption, or we get drives with
        //      PETABYTES of storage, i will fix it.
        size_t inode_offset = offset*inode_size;
        size_t inode_addr = inode_table_base + inode_offset;
        size_t inode_block = inode_addr/block_size;
        size_t inode_block_offset = inode_addr%block_size;
        read_block(inode_block,block_size);
        kmemcpy(ubuffer,buffer+inode_block_offset,inode_size);
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
        // BUG: THIS WILL NOT WORK WITH DIRECTORIES THAT NEED MORE THAN 12 BLOCKS. THIS COULD BREAK WITH AS FEW AS 3 CHILDREN.
        Inode parent;
        get_inode(&parent,parentAddress,superblock,bgd_table);
        Terminal::print("Parent addr: ");
            Terminal::print(parseInt(parent.types_permissions,throwawayString,16));
            Terminal::print("\n");
        if (parent.types_permissions == 0) {
            // fault(-404,"get_child of inode with no type or permissions");
        }
        for (size_t block = 0; block < 12; block++) {
            Terminal::print("BBBBBBBBBBBBb\n");
            if (parent.direct_pointers[block] == 0) continue;
            ExtState response = read_block(parent.direct_pointers[block],block_size);
            if (response != SUCCESS) {
                return {response,0};
            }
            DirectoryEntry* directories = (DirectoryEntry*)buffer;
            size_t offset = 0;
            DirectoryEntry* currentDirectory;
            while (offset < block_size) {
                Terminal::print("AAAAAAAAa\n");
                currentDirectory = (DirectoryEntry*)((ubyte_t*)directories + offset);
                if (currentDirectory->inode != 0) {
                    Terminal::print("inode name_len total_size name exp_name: ");
                        Terminal::print(parseInt(currentDirectory->inode,throwawayString,16));
                        Terminal::print(" ");
                        Terminal::print(parseInt(currentDirectory->name_len,throwawayString,16));
                        Terminal::print(" ");
                        Terminal::print(parseInt(currentDirectory->total_size,throwawayString,16));
                        Terminal::print(" ");
                        Terminal::print((currentDirectory->name));
                        Terminal::print(" ");
                        Terminal::print((name));
                        Terminal::print("\n");
                    if (currentDirectory->name_len == strlen(name) and
                        kmemcmp(name,currentDirectory->name,currentDirectory->name_len) == 0) {
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
     * Reads a block from an Inode.
     * 
     * BUG: This does not work for the 13th block or above, because indirect blocks have not been handled.
     * 
     * Returns:
     * - SUCCESS
     * - UNALIGNED (blockSize is not aligned to the sector size.)
     * - LARGE (blockSize is larger than the maximum allowed block size.)
     */
    ExtState read_inode_block(size_t blockIndex, Inode* inode, Superblock* superblock) {
        if (blockIndex >= 1036) {
            fault(2,"LAZY (ext2.read_inode_block)");
        } else if (blockIndex >= 12) {
            size_t block = inode->pointer_single_indirect;
            read_block(block,block_size);
            block = ((ubyte4_t*)buffer)[blockIndex-12];
            read_block(block,block_size);
            return SUCCESS;
        } else {
            size_t block = inode->direct_pointers[blockIndex];
            read_block(block,block_size);
            return SUCCESS;
        }
        return FAILURE;
    }
    /**
     * Finds an inode with a given path on the disk.
     * 
     * Returns:
     * - Pair<ExtState,DirectoryEntry>
     * 
     * ExtState:
     * - SUCCESS
     * - UNALIGNED (blockSize is not aligned to the sector size.)
     * - LARGE (blockSize is larger than the maximum allowed block size.)
     * - BAD_PATH (path is formatted incorrectly.)
     */
    Pair<ExtState,ubyte4_t> find_path(char* path,Superblock* superblock, BlockGroupDescriptor* bgd_table) {
        // BUG: fails to parse the whole tree
        if (path[0] != '/') {
            while(true) {}
            return {BAD_PATH,NULL};
        }
        size_t parent = 2;
        Pair<ext2::ExtState,ubyte4_t> child = {SUCCESS,0};
        char buf[255];
        kmemset(buf,0,255);
        size_t bufidx = 0;
        Terminal::print(path);
            Terminal::print("\n");
        for (size_t index = 1;true;index++) {
            Terminal::print(buf);
                Terminal::print("\n");
            if (path[index] == '/' or path[index] == '\0') {
                child = get_child(buf,parent,superblock,bgd_table);
                if (child.first != ext2::SUCCESS) {
                    while (true){}
                    if (child.first == ext2::NOT_FOUND) {
                        fault(1,buf,"NOT FOUND");
                    }
                    fault(1,buf);
                    while(true) {}
                    return child;
                }
                parent = child.second;
                Terminal::print("parent: ");
                    Terminal::print(parseInt(child.second,throwawayString,16));
                    Terminal::print("\n");
                kmemset(buf,0,255);
                bufidx = 0;
            } else {
                buf[bufidx] = path[index];
                buf[bufidx+1] = 0;
                bufidx++;
                continue;
            }
            if (path[index] == '\0') {
                while(true) {}
                return {SUCCESS,parent};
            }
        }
        while(true) {}
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