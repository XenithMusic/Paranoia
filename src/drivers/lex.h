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
    scheduler::Process* loadLEXIntoMemory(char* path,pid_t parent);
}