#include "types.h"
#include "memory.h"
#include "vector.h"
#include "paranoia.h"
#include "utils.h"

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
    __attribute__((noreturn))
    void enterProcess(volatile Process* process) {
        __asm__ __volatile__(
            "movl %0, %%esp\n"
            "popal\n"
            "pop %%ds\n"
            "pop %%es\n"
            "pop %%fs\n"
            "pop %%gs\n"
            "iret\n"
            "hlt" : : "r"(&process->ctx.ax) : "memory"
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