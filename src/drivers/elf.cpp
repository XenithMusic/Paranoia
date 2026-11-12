#include "scheduler.h"
#include "types.h"
#include "drivers/vfs.h"

#define IDENT_CLASS 4
#define IDENT_DATA 5
#define IDENT_VERSION 6

#define CLASS_NONE 0
#define CLASS_32BIT 1
#define CLASS_64BIT 2

#define DATA_NONE 0
#define DATA_LSB 1
#define DATA_MSB 2

const unsigned char ELF_MAGIC[4] = { 0x7f, 'E', 'L', 'F' };

namespace ELFLoader {
    scheduler::Process* loadELFIntoMemory(char* path) {
        VFS::File file = VFS::getFileFromPath(path);
        fid_t fid = VFS::openFile(file);
        if (VFS::getFileSize(fid) < sizeof(ELFHeader)) {
            VFS::closeFile(fid);
            return nullptr;
        }
        ELFHeader header;
        VFS::readFile(fid,0,sizeof(ELFHeader),&header);
        if (kmemcmp(ELF_MAGIC,header.ident,4) != 0) {
            VFS::closeFile(fid);
            return nullptr; // invalid elf executable
        }
        if (header.ident[IDENT_CLASS] != CLASS_32BIT) {
            VFS::closeFile(fid);
            return nullptr; // this is a 32-bit kernel, we cannot run 64-bit code
        }
        // EI_DATA check. don't immediately understand this so i need to write this after i finish
        if (header.ident[IDENT_DATA] != DATA_LSB) {
            VFS::closeFile(fid);
            return nullptr; // i THINK this is the correct mode for the architecture i'm compiling for
        }
        if (header.ident[IDENT_VERSION] != header.version) {
            VFS::closeFile(fid);
            return nullptr;
        }
        // ...todo..
    }
}