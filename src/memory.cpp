#include <stddef.h>
#include "types.h"
#include "terminal.h"
#include "string.h"
#include "const.h"
#include "fail.h"
#include "math.h"

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

    static char* memory_pool = (char*)MEM_START;
	static MBlock bitmap[NUM_BLOCKS];

    void init() {
        if ((NUM_BLOCKS%1) != 0) {
            setError(-2032);
            fault(-101);
        }
        if ((CONST_KERNELSIZE) >= NUM_BLOCKS) {
            setError(-2033);
            fault(-101);
        }
    	for (int i = 0; i < NUM_BLOCKS; i++) {
    		bitmap[i].state = FREE;
            bitmap[i].isEnd = true;
            if (i < CONST_KERNELSIZE) {
                bitmap[i].state = RSRV;
                // bitmap[i].isEnd = true; -- This is redundant.
                if (i != 0) {
                    bitmap[i-1].isEnd = false;
                }
            }
    	}
    }

    size_t getBlocks() {
        return NUM_BLOCKS;
    }

    size_t getBlockSize() {
        return BLOCK_SIZE;
    }

    void *malloc(size_t size) {
        size_t blocksToAllocate = 1;
    	if (size > BLOCK_SIZE) {
            setError(-2028);
            blocksToAllocate = (size_t)ceil(size/BLOCK_SIZE);
            // will allow such an allocation, despite error -2028.
            // the error here acts more as a warning.
        }

        size_t contiguousBlocks = 0;
        size_t start_block = 0;

        for (int i = 0; i < NUM_BLOCKS; i++) {
            if (bitmap[i].state == FREE) {
                if (contiguousBlocks == 0) {
                    start_block = i;
                }
                contiguousBlocks++;
            } else {
                contiguousBlocks = 0;
                start_block = 0;
            }
            if (contiguousBlocks >= blocksToAllocate) {
                for (size_t j = start_block; j < start_block + blocksToAllocate; j++) {
                    bitmap[j].state = USED;
                    bitmap[j].isEnd = (j == start_block + blocksToAllocate - 1);
                }
                return (void*)(memory_pool + start_block * BLOCK_SIZE);
            }
        }

        setError(-2026);

        return nullptr;
    }

    void free(void* pointer) {
        size_t index = ((char*)pointer - memory_pool) / BLOCK_SIZE;
        if (index < CONST_KERNELSIZE) {
            setError(-2030);
            return;
        }
        if (bitmap[index].state == RSRV) {
            setError(-2030);
            return;
        }
        if (bitmap[index].state == FREE) {
            setError(-2031);
            return;
        }
        if (index >= NUM_BLOCKS) {
            setError(-2029);
            return;
        }

        // Free the segmented block.

        for (size_t j = index; j < NUM_BLOCKS; j++) {
            bitmap[j].state = FREE;
            if (bitmap[j].isEnd) {
                return;
            }
        }
        setError(-100);
        fault(-100);
        return;
    }
}