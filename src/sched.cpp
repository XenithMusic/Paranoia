#include "types.h"
#include "utils.h"
#include "math.h"
#include "drivers/ext2.h"
#include "fail.h"

namespace scheduler {
    Process processes[1024] = {};
    uint16_t index = 0;
    size_t cycles = 0;
    Process kernel = {};
    void init() {
        // any important init goes here
    }
    void ctxsw(Process* target) {
        __asm__ __volatile__(
            "mov %0, %%cr3"
            :: "r"(target->CR3)
        );
        __asm__ __volatile__(
            "mov %0, %%eax\n"
            "mov %1, %%ebx\n"
            "mov %2, %%ecx\n"
            "mov %3, %%edx\n"
            "mov %4, %%esi\n"
            "mov %5, %%edi\n"
            :: "r"(target->EAX),"r"(target->EBX),"r"(target->ECX),
            "r"(target->EDX),"r"(target->ESI),"r"(target->EDI)
        );
        __asm__ __volatile__(
            "mov %0, %%ebp\n"
            "mov %1, %%ds\n"
            "mov %2, %%es\n"
            "mov %3, %%fs\n"
            "mov %4, %%gs\n"
            :: "r"(target->EBP),"r"(target->DS),"r"(target->ES),
            "r"(target->FS),"r"(target->GS)
        );
        __asm__ __volatile__(
            "push %0\n" // ss
            "push %1\n" // esp
            "push %2\n" // eflags
            "push %3\n" // cs
            "push %4\n" // eip
            "iret" // NOTE: CONTEXT SWITCH!!!
            :: "r"(target->SS),"r"(target->ESP),"r"(target->EFLAGS),
            "r"(target->CS),"r"(target->EIP)
        );
    }
    void tick() {
        cli();
        
        __asm__ __volatile__ (
            "mov %%eax, %0\n"
            "mov %%ebx, %1\n"
            "mov %%ecx, %2\n"
            "mov %%edx, %3\n"
            "mov %%esi, %4\n"
            : "=r"(kernel.EAX), "=r"(kernel.EBX), "=r"(kernel.ECX), "=r"(kernel.EDX),
            "=r"(kernel.ESI)
        ); // store registers
        __asm__ __volatile__(
            "mov %%edi, %0\n"
            "mov %%ebp, %1\n"
            : "=r"(kernel.EDI), "=r"(kernel.EBP)
        );

        kernel.ESP = (uint32_t)__builtin_frame_address(0);

        __asm__ __volatile__ (
            "mov %%ds, %0\n"
            "mov %%es, %1\n"
            "mov %%fs, %2\n"
            "mov %%gs, %3\n"
            : "=r"(kernel.DS), "=r"(kernel.ES), "=r"(kernel.FS), "=r"(kernel.GS)
        ); // store segments

        __asm__ __volatile__ (
            "pushf\n"
            "pop %0\n"
            : "=r"(kernel.EFLAGS)
        ); // yummy flags mmm tasty

        __asm__ __volatile__ (
            "mov %%cr3, %0\n"
            : "=r"(kernel.CR3) 
        ); // yummy CR registers

        ctxsw(&processes[index]);
        fault(-900,nullptr,"sched.scheduler::tick"); // this should never be reached, if it is, something went horribly wrong.
    }
    void createProcess(uint32_t entry_code, uint32_t entry_stack,
        uint16_t iCS, uint16_t iDS, uint16_t iSS, uint32_t iCR3,
        uint32_t iRFLAGS) {
        uint16_t i;
        for (i = 0;processes[i].valid == false; i++) {
            if (i == 1023) {
                return;
            }
        }
        if (i == 1023) return;
        processes[i] = {
            valid:true,
            EAX:0, EBX:0, ECX:0, EDX:0, ESI:0, EDI:0,
            EBP:0, ESP:entry_stack, EIP:entry_code,
            CS:iCS, DS:iDS, SS:iSS, ES:0, FS:0, GS:0,EFLAGS:iRFLAGS,
            CR3:iCR3
        };
    }
}
namespace process {
    ext2::Superblock* superblock;
    void load_bin(void* dest, ext2::Inode* src, size_t dest_size) {
        // NOTE: add this back
        // ext2::get_superblock(superblock);
        // size_t block_size = 1024 << (superblock->block_size);
        // size_t file_size = src->lower_size | (src->upper_size << 32);
        // for (size_t seek = 0;seek < min(dest_size,file_size);seek+=block_size) {
        //     ext2::read_inode_block(seek/block_size,src,superblock); // read the next bit of data
        //     ext2::get_buffer(min(min(block_size,file_size),(dest_size-seek)),dest,seek); // append the buffer to `dest`.
        // }
        return;
    }
}