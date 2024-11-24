#include <stddef.h>
#include "types.h"
#include "terminal.h"
#include "string.h"
#include "const.h"
#include "fail.h"

#define MEMORY_POOL_SIZE 512*MEBIBYTES // 512 MiB
#define BLOCK_SIZE 512 // 512B
#define NUM_BLOCKS MEMORY_POOL_SIZE/BLOCK_SIZE // 512 MiB

#define MEM_START (0x1*MEBIBYTES)

#if (NUM_BLOCKS%1) != 0
#error "(-2032) NUM_BLOCKS should be an integer."
#endif

#if CONST_KERNELSIZE >= NUM_BLOCKS
#error "(-2033) CONST_KERNELSIZE cannot exceed the available memory pool."
#endif

namespace Allocator {

    void init();

    void *malloc(size_t size);

    void free(void* pointer);
}