#include "ata.h"
#include "const.h"
#include "memory.h"
#include "types.h"

// How large sectors returned by the Disk Driver are.
#define SECTOR_SIZE 512

// Byte offset that the ext2 filesystem starts at.
#define FS_OFF 16*MEBIBYTES

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
    ExtState get_buffer(size_t bufferSize, void* userBuffer);
    ExtState get_superblock(Superblock* userBuffer);
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
    ExtState read_block(size_t blockNo,size_t blockSize);
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
    ExtState read_block(size_t blockNo, size_t blockSize, void* userBuffer);
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
    ExtState read_superblock(size_t blockSize, Superblock* userBuffer);
    /**
     * Gets the size of the Block Group Descriptor Table in bytes.
     */
    size_t get_bgdt_size(Superblock* superblock);
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
    ExtState read_bgd_table(BlockGroupDescriptor* userBuffer,Superblock* superblock);
    /**
     * Returns the Inode Table.
     * 
     * Address is in blocks.
     * 
     * Returns:
     * - Inode* always.
     */
    Inode* get_inode_table(size_t address,Superblock* superblock,BlockGroupDescriptor* bgd_table);
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
    Inode* get_inode(size_t address,Superblock* superblock,BlockGroupDescriptor* bgd_table);
    /**
     * Reads a block an Inode.
     * 
     * BUG: This does not work for the 13th block or above, because indirect blocks have not been handled.
     * 
     * Returns:
     * - SUCCESS
     * - UNALIGNED (blockSize is not aligned to the sector size.)
     * - LARGE (blockSize is larger than the maximum allowed block size.)
     */
    ExtState read_inode_data(size_t blockIndex, Inode* inode, Superblock* superblock);

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
    Pair<ExtState,ubyte4_t> get_child(char name[], size_t parentAddress,Superblock* superblock, BlockGroupDescriptor* bgd_table);
    ExtState init();
}