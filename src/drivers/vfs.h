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
namespace VFS {
    void init();
    fsid_t mount(FilesystemType fsType, disk::DriveType driveType, uint16_t driveIndex, uint16_t partitionIndex,
        char* identifierOverride);
    bool unmount(fsid_t fsid);
    Vector<char*> splitPath(char* path);
    fsid_t getAssociatedFilesystem(Vector<char*>& splitPath);
    char* getTargetPath(char* path);
    File getFileFromPath(char* path);
    fid_t openFile(File file);
    void closeFile(fid_t fid);
    void readFile(fid_t fid, size_t startAddress, size_t bytes, void* buffer);
}