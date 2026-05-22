#include "types.h"
#include "vector.h"
#include "drivers/ext2.h"

/*
Paths are stored as a unix-like path string "/blah/blah/blah.txt"

The first item in the path (/first/second/third) is an identifier for the filesystem.
Everything following is passed down to the target filesystem.
Everything after the identifier is referred to as a targetPath.
This name may change later in development if I think of a better alternative.
*/

// do not understand these two operator definitions but apparently i get errors without these
// something something freestanding environment something something nostdlib

namespace VFS {
    Vector<Filesystem*> filesystems;
    Vector<OpenFile*> openedfiles;
    void init() {
        filesystems.open();
        openedfiles.open();
    }
    fsid_t mount(FilesystemType fsType, disk::DriveType driveType, uint16_t driveIndex, uint16_t partitionIndex,
    char* identifierOverride) {
        Filesystem* constructed = (Filesystem*)Allocator::kalloc(sizeof(Filesystem));
        disk::Filesystem* fs;
        if (fsType == ext2) {
            fs = ext2::mount(driveType,driveIndex,partitionIndex);
            if (fs == nullptr) {
                ext2::unmount(fs);
                return -1;
            }
        } else {
            return -1;
        }
        constructed->type = fsType;
        constructed->fs = fs;
        kmemset(constructed->fs->identifier,0,256);
        if (identifierOverride != nullptr) {
            kmemcpy(constructed->fs->identifier,identifierOverride,strlen(identifierOverride));
        } else {
            char* parsedDriveIdx = (char*)Allocator::kalloc(32);
            char* parsedPartIdx = (char*)Allocator::kalloc(32);
            kmemset(parsedDriveIdx,0,32);
            kmemset(parsedPartIdx,0,32);
            parseInt(driveIndex,parsedDriveIdx,16);
            parseInt(partitionIndex,parsedPartIdx,16);
            size_t diStrLen = strlen(parsedDriveIdx);
            size_t piStrLen = strlen(parsedPartIdx);
            kmemcpy(constructed->fs->identifier,"disk",4);
            kmemcpy(constructed->fs->identifier+4,parsedDriveIdx,diStrLen);
            kmemcpy(constructed->fs->identifier+4+diStrLen,"p",1);
            kmemcpy(constructed->fs->identifier+4+diStrLen+1,parsedPartIdx,piStrLen);
            Terminal::printdebug("(vfs::mount) new drive identifier is ");
                Terminal::printdebug(constructed->fs->identifier);
                Terminal::printdebug("\n");
        }
        (filesystems).add(constructed);
        return filesystems.length-1;
    }
    bool unmount(fsid_t fsid) {
        if ((filesystems)[fsid]->type == ext2) {
            ext2::unmount((filesystems)[fsid]->fs);
        }
        (filesystems)[fsid]->valid = false;
        return true;
    }
    Vector<char*> splitPath(char* path) {
        size_t pathLen = strlen(path);
        if (pathLen <= 1) return {};
        Vector<char*> returnValue = {};
        returnValue.open();
        int i = 0;
        int pathItemLen = 0;
        bool shouldExit = false;
        while (path[i] != 0 && i < 256 && !shouldExit) {
            i++;
            if (path[i] == 0) shouldExit = true;
            if (path[i] == '/' || path[i] == 0) {
                Terminal::printdebug(" ::: found split at index ");
                    Terminal::printdebug(parseInt(i,10));
                    Terminal::printdebug(", with content ");
                    Terminal::printdebug(path+i-pathItemLen);
                    Terminal::printdebug("\n");
                returnValue.add(path+i-pathItemLen);
                pathItemLen = 0;
                continue;
            }
            pathItemLen++;
        }
        for (int i=0;i<pathLen;i++) {
            if (path[i] == '/') path[i] = 0;
        }
        if (i == 256) Terminal::print("\n!!!\n!!!\nhit path length limit while splitting path\n");
        return returnValue;
    }
    fsid_t getAssociatedFilesystem(Vector<char*>& splitPath) {
        size_t splitlen = strlen(splitPath[0]);
        fsid_t i = 0;
        Terminal::printdebug("(getting associated filesystems)\n");
        for (VFS::Filesystem* fs : (filesystems)) {
            Terminal::printdebug("... ");
                Terminal::printdebug(fs->fs->identifier);
                Terminal::printdebug(" ");
                Terminal::printdebug(splitPath[0]);
                Terminal::printdebug(" ");
                Terminal::printdebug(parseInt(splitlen,10));
                Terminal::printdebug(" ");
                Terminal::printdebug(parseInt(strlen(fs->fs->identifier),10));
                Terminal::printdebug("\n");
            if (splitlen == strlen(fs->fs->identifier) && kmemcmp(fs->fs->identifier,splitPath[0],splitlen) == 0) {
                return i;
            }
            i++;
        }
        Terminal::print("(failed)\n");
        while(1){}
        return -1;
    }
    char* getTargetPath(char* path) {
        int i = 1;
        while (path[i] != '/') {
            i++;
        }
        return path;
    }
    File getFileFromPath(char* path) {
        Terminal::printdebug("splitting path\n");
        Vector<char*> split = splitPath(path);
        fsid_t fs = getAssociatedFilesystem(split);
        if (fs == -1) fault(10000,"fs == -1");
        Terminal::printdebug("getting target path\n");
        char* targetPath = getTargetPath(path);
        Terminal::printdebug("constructing file object\n");
        File constructed;
        constructed.filesystem = fs;
        constructed.valid = true;
        Terminal::printdebug("getting filesystem\n");
        Filesystem* fsentry = (filesystems)[constructed.filesystem];
        if (fsentry->valid == false) return {};
        Pair<bool,ext2::DirectoryMetadata> dir;
        split.remove(0);
        Terminal::printdebug("entering loop\n");
        if (fsentry->type == ext2) {
            ext2::Inode* inode = ext2::open_inode(fsentry->fs,2);
            for (char* item : split) {
                Terminal::printdebug("-- parsing path at '");
                    Terminal::printdebug(item);
                    Terminal::printdebug("'\n");
                dir = ext2::get_child(fsentry->fs,inode,item);
                ext2::close_inode(inode);
                inode = ext2::open_inode(fsentry->fs,dir.second.inode);
            }
            ext2::close_inode(inode);
        }
        Terminal::printdebug("done looping\n");
        constructed.details.ext2entry = dir.second;
        constructed.file = dir.second.inode;
        return constructed;
    }
    fid_t openFile(File file) {
        Filesystem* fsentry = (filesystems)[file.filesystem];
        OpenFile* constructed = (OpenFile*)Allocator::kalloc(sizeof(OpenFile));
        constructed->file = file;
        if (fsentry->type == ext2) {
            constructed->data.ext2 = ext2::open_inode(fsentry->fs, constructed->file.details.ext2entry.inode);
        }
        size_t outputFid = openedfiles.length;
        for (size_t i=0;i<openedfiles.length;i++) {
            if (openedfiles[i] == nullptr) {
                openedfiles[i] = constructed;
                outputFid = i;
                goto foundFreeSlot;
            }
        }
        openedfiles.add(constructed);
        foundFreeSlot:
        return outputFid;
    }
    void closeFile(fid_t fid) {
        OpenFile* file = openedfiles[fid];
        Filesystem* fsentry = (filesystems)[file->file.filesystem];
        if (fsentry->type == ext2) {
            ext2::close_inode(file->data.ext2);
        }
        Allocator::free(file);
        openedfiles[fid] = nullptr;
    }
    void readFile(fid_t fid, size_t startAddress, size_t bytes, void* buffer) {
        OpenFile* file = openedfiles[fid];
        Filesystem* fsentry = (filesystems)[file->file.filesystem];
        if (fsentry->type == ext2) {
            ext2::read_inode(fsentry->fs,file->data.ext2,startAddress,bytes,buffer);
        }
    }
    uint64_t getFileSize(fid_t fid) {
        OpenFile* file = openedfiles[fid];
        Filesystem* fsentry = (filesystems)[file->file.filesystem];
        uint64_t size = 0;
        if (fsentry->type == ext2) {
            size = file->data.ext2->lower_size;
        }
        return size;
    }
    uint64_t getDiskAllocationSize(fid_t fid) {
        OpenFile* file = openedfiles[fid];
        Filesystem* fsentry = (filesystems)[file->file.filesystem];
        uint64_t size = getFileSize(fid);
        uint64_t realsize = size;
        if (fsentry->type == ext2) {
            size_t blockSize = 1024<<fsentry->fs->fs.ext2->superblock->block_size;
            realsize += ((size+blockSize-1)/blockSize)/8;
            // the above is BGDT block bitmap contribution. this may seem a little odd, since this is a 
            // filesystem-wide concept, however this is always true, as this will add no blocks for 0-byte files, matching
            // function behavior, and add one block (one bit) for 1-byte files, which would match the growth of the 
            // inode bitmap table; the inode bitmap table is incredibly unlikely to force an expansion of the BGDT,
            // thus this addition holds true.
            //
            // if this is not odd, my bad, i told an AI how i was doing this (i wrote the code) and it more or
            // less asked me "why the fuck" and said this was bullshit.

            realsize += fsentry->fs->fs.ext2->superblock->inode_size; // size of the inode entry.
        }
        if (realsize < size) {
            realsize = size; // WOW, THAT'S A BIG ASS FILE. 9 EXABYTES?? ARE YOU ON LIKE ZFS OR SOME SHIT???
        }
        return realsize;
    }
}