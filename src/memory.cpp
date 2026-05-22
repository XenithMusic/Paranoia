#include <stddef.h>
#include "types.h"
#include "terminal.h"
#include "string.h"
#include "const.h"
#include "fail.h"

/*

Copyright (C) 2026  XenithMusic (on github)

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
            fault(-101);
        }
        if ((CONST_KERNELSIZE) >= NUM_BLOCKS) {
            fault(-101);
        }
        for (size_t i = 0;i < NUM_BLOCKS;i++) {
            bitmap[i].isEnd = true;
            bitmap[i].state = FREE;
            if (i < CONST_KERNELSIZE) {
                bitmap[i].state = RSRV;
                // bitmap[i].isEnd = true; -- This is redundant.
                if (i != 0) {
                    bitmap[i-1].isEnd = false;
                }
            }
        }
    }
    bool is_allocated(void* ptr) {
        size_t startBlock = (((size_t)ptr)-MEM_START)/BLOCK_SIZE;
        MBlock* block = &bitmap[startBlock];
        return block->state != MBlockState::FREE;
    }
    void *kalloc(size_t size) {
        size_t contiguous = 0;
        size_t needed = (size+BLOCK_SIZE-1)/BLOCK_SIZE; // FATAL: INCOMPLETE IMPL -- HOW SO WHAT AM I MISSING -- FOUND IT LOL
        size_t start = SIZE_MAX;
        Terminal::printdebug("(kalloc) I need ");
            Terminal::printdebug(parseInt(needed,10));
            Terminal::printdebug(" blocks!\n");
        if (needed == 0) {
            return nullptr;
        }
        for (size_t i = 0;i < NUM_BLOCKS;i++) {
            if (bitmap[i].state != FREE) {
                start = SIZE_MAX;
                contiguous = 0;
                continue;
            }
            if (start == SIZE_MAX) start = i;
            contiguous++;
            if (contiguous >= needed) {
                for (size_t j = start;j <= i;j++) {
                    bitmap[j].state = USED;
                    bitmap[j].isEnd = false;
                }
                bitmap[i].isEnd = true;
                void* ptr = (void*)(MEM_START+start*BLOCK_SIZE);
                return (void*)ptr;
            }
        }
        fault(-102);
        return nullptr;
    }

    void free(void* pointer) {
        size_t startBlock = (((size_t)pointer)-MEM_START)/BLOCK_SIZE;
        MBlock* block = &bitmap[startBlock];
        if (block->state != MBlockState::USED) return;
        bool isEnd = false;
        while (!block->isEnd) {
            if (block->state != MBlockState::USED) fault(-100,"Found unused block inside allocation while freeing.");
            block->state = MBlockState::FREE;
            block += 1;
        }
    }
}

void kmemset(void* start, char value, size_t length) {
    char* startChar = (char*)start;
    for (size_t i = 0; i < length; i++) {
        startChar[i] = value;
    }
}

int kmemcmp(const void* s1, const void* s2, size_t n) {
    const uint8_t* p1 = (const uint8_t*)s1;
    const uint8_t* p2 = (const uint8_t*)s2;

    for (size_t i = 0; i < n; i++) {
        if (p1[i] < p2[i]) return -1;
        if (p1[i] > p2[i]) return 1;
    }
    return 0;
}

void* kmemcpy(void* dest, const void* src, size_t n) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }
    return dest;
}

// kmemcpy but it's safe for overlapping regions
void* kmemmove(void* dest, const void* src, size_t n) {
    uint8_t* swap = (uint8_t*)Allocator::kalloc(n); 
    kmemcpy(swap,src,n);
    uint8_t* d = (uint8_t*)dest;
    for (size_t i = 0; i < n; i++) {
        d[i] = swap[i];
    }
    return dest;
}