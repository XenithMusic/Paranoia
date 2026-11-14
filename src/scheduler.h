#include "types.h"

namespace scheduler {
    void init();
    void tick();
    void addProcess(Process* process);
    void enterProcess(volatile Process* process);
    SyscallArguments* getSysargs();
    SyscallReturns* getSysrets();
}