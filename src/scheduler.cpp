#include "types.h"
#include "memory.h"
#include "vector.h"
#include "paranoia.h"

#define QUANTUM_LENGTH 1000 // 1 millisecond :3

namespace scheduler {
    SyscallArguments* sysargs = nullptr;
    SyscallReturns* sysrets = nullptr;
    Vector<Process*> processes;
    size_t lastProcess = 0;
    void init() {
        processes.open();
        sysargs = (SyscallArguments*)Allocator::kalloc(0xFFFF);
        sysrets = (SyscallReturns*)Allocator::kalloc(0xFFFF);
    }
    void setupTimer(size_t quantumMicroseconds) {

    }
    void enterProcess(Process* process) {
        Terminal::print(parseInt(process->ctx.ss,16));
        __asm__ volatile(
            "mov %0, %%eax \n"
            "mov %1, %%ebx \n"
            "mov %2, %%ecx \n"
            "mov %3, %%edx \n"
            "mov %4, %%esi \n"
            "mov %5, %%edi \n"
            "mov %6, %%ebp \n"
            
            "pushl %7 \n"
            "pushl %8 \n"
            "pushl %9 \n"
            "pushl %10 \n"
            "pushl %11 \n"
            "iret \n"
            :: "m"(process->ctx.eax),
               "m"(process->ctx.ebx),
               "m"(process->ctx.ecx),
               "m"(process->ctx.edx),
               "m"(process->ctx.esi),
               "m"(process->ctx.edi),
               "m"(process->ctx.ebp),

               "m"(process->ctx.ss),
               "m"(process->ctx.esp),
               "m"(process->ctx.eflags),
               "m"(process->ctx.cs),
               "m"(process->ctx.eip)
               : "eax","ebx","ecx","edx","esi"
        );
        while(1){}
    }
    void tick() {
        if (processes.length == 0) AfterAllProcesses();
        lastProcess = (lastProcess+1)%processes.length;
        Process* process = processes[lastProcess];
        process->ticksSinceRun++;
        if (process->ticksSinceRun >= process->ticksPerRun) {
            process->ticksSinceRun = 0;
            setupTimer(QUANTUM_LENGTH);
            enterProcess(process);
        }
        Terminal::print("BAM!!!\n");
        __asm__ volatile("hlt");
    }
    void addProcess(Process* process) {
        processes.add(process);
    }
    SyscallArguments* getSysargs() {
        return sysargs;
    }
    SyscallReturns* getSysrets() {
        return sysrets;
    }
}