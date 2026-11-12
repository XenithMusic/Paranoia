#include "types.h"
#include "memory.h"
#include "drivers/vfs.h"
#include "page.h"
#include "scheduler.h"

#define STACK_SIZE 262144 // 256KiB

/**
 * Code for a custom executable format I made specifically for testing!
 */
namespace LEXLoader {
    scheduler::Process* loadLEXIntoMemory(char* path,pid_t parent) {
        LEXHeader* header = (LEXHeader*)Allocator::kalloc(1024);
        VFS::File file = VFS::getFileFromPath(path);
        fid_t fid = VFS::openFile(file);
        VFS::readFile(fid,0,1024,header);
        uint8_t* processData = nullptr;
        scheduler::Process* process = (scheduler::Process*)Allocator::kalloc(sizeof(scheduler::Process));
        if (kmemcmp(header->magic,"LEX\x42",4) != 0) {
            return nullptr; // invalid executable
        }
        if (header->architecture != X86_32) {
            return nullptr; // wrong arch
        }
        if (header->endianness != LSB) {
            return nullptr; // wrong endianness
        }
        if (header->version != 3) {
            return nullptr; // we dont support this version
        }
        processData = (uint8_t*)Allocator::kalloc(header->expectedMemorySize+STACK_SIZE);
        // load all memory regions
        process->pageDirectory = (PageDirectory*)Allocator::kalloc(sizeof(PageDirectory));
        kmemset(process->pageDirectory,0,sizeof(PageDirectory));
        size_t offsetFile = header->headerSize;
        size_t offsetProcess = STACK_SIZE;
        size_t virtualOffset = header->expectedBase;
        LEXMemSection* section = (LEXMemSection*)Allocator::kalloc(header->memSectionSize);
        Paging::mapArbitraryRange(process->pageDirectory,virtualOffset,(uint32_t)processData,STACK_SIZE,0b111); // stack
        for (size_t i = 0;i < header->memorySections;i++) {
            VFS::readFile(fid,offsetFile,header->memSectionSize,section);
            size_t alignment = 4096-((section->size+section->paddingAfter)%4096);
            if (alignment != 4096) section->paddingAfter += alignment;
            size_t sectionStart = offsetFile+header->memSectionSize;
            uint8_t* sectionTarget = processData+offsetProcess;
            if ((section->flags&0x1) != 0) {
                kmemset(sectionTarget,0,section->size+section->paddingAfter);
            }
            VFS::readFile(fid,sectionStart,section->size,processData+offsetProcess);
            uint16_t flags = 0b000000000001;
            flags |= section->pageFlags;
            Paging::mapArbitraryRange(process->pageDirectory,virtualOffset,(uint32_t)sectionTarget,section->size+section->paddingAfter,flags);
            offsetFile += section->size + header->memSectionSize;
            offsetProcess += section->size + section->paddingAfter;
            virtualOffset += section->size + section->paddingAfter;
        }
        Terminal::printdebug("(lexloader) LOADED TO ");
            Terminal::printdebug(parseInt((uint32_t)processData,16));
            Terminal::printdebug("\n");
        Paging::mapArbitraryRange(process->pageDirectory,0x1000000,(uint32_t)scheduler::getSysargs(),0xFFFF,0b000000000111); // user RW
        Paging::mapArbitraryRange(process->pageDirectory,0x1010000,(uint32_t)scheduler::getSysrets(),0xFFFF,0b000000000101); // user R
        if (header->linkerMode == STATIC) {
        } else {
            return nullptr; // TODO: support dynamic linking
        }

        process->entrypoint = (void*)header->entrypoint;
        process->memoryBase = processData;
        process->parentPid = parent;
        process->ticksPerRun = 1;
        process->ticksSinceRun = 0;
        {
            kmemset(&process->ctx,0,sizeof(scheduler::ProcessContext));
            process->ctx.esp = (uint32_t)process->memoryBase+STACK_SIZE;
            process->ctx.eip = (uint32_t)process->entrypoint;
            process->ctx.eflags = 0x202;
            process->ctx.cs = 0x18+3;
            process->ctx.ss = 0x20+3;
        }

        Allocator::free(header);
        Allocator::free(section);

        return process;
    }
}