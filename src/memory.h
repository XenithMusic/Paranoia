#include <stddef.h>
#include "types.h"
#include "terminal.h"
#include "string.h"
#include "const.h"
#include "fail.h"

/*

Copyright (C) 2024  XenithMusic (on github)

The Paranoia kernel is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Paranoia is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Paranoia. If not, see <https://www.gnu.org/licenses/>.

*/


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