#include "types.h"

namespace ext2 {
    disk::Filesystem* mount(disk::DriveType driveType, uint16_t driveIndex, uint16_t partitionIndex);
    void unmount(disk::Filesystem* fs);
    Inode* get_inode(disk::Filesystem filesystem, Superblock* superblock, BlockGroupDescriptor* bgdt, size_t address);
    Inode* open_inode(disk::Filesystem* fs, size_t address);
    void close_inode(Inode* inode);
    void read_inode(disk::Filesystem* fs,Inode* inode, size_t startAddress, size_t bytes, void* buffer);
    void read_inode_block(disk::Filesystem* fs,Inode* inode, size_t block, void* buffer);
    void read_block(disk::Filesystem* fs,size_t blockIndex,void* buffer);
    Pair<bool,DirectoryMetadata> get_child(disk::Filesystem* fs, Inode* inode, char* child_name);
}