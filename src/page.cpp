#include "types.h"
#include "stdint.h"
#include "fail.h"
#include "memory.h"

/*

Copyright (C) 2026  XenithMusic (on github)

The Paranoia kernel is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Paranoia is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Paranoia. If not, see <https://www.gnu.org/licenses/>.

*/

#define PAGE_TABLE_COUNT 128

namespace Paging {
    bool enabled = false;
    // the basic page directory for the kernel
    static PageDirectory kernelPD alignas(4096);

    // allocates 12K, and rounds the pointer up to the next 4K boundary.
    void* kalloc4K() {
        uintptr_t pointer = (uintptr_t)Allocator::kalloc(1024*12);
        uintptr_t aligned = (pointer+0xFFF)&(~0xFFF);
        return (void*)aligned;
    }

    void invalidatePage(uint32_t phys) {
        asm volatile("invlpg (%0)" : : "r"(phys) : "memory");
    }

    void mapIdentityPage(PageDirectory* pd, uint32_t phys) {
        uint32_t dirIndex = (phys>>22) & 0x3FF;
        uint32_t tblIndex = (phys>>12) & 0x3FF;

        PageTable* pt;

        if (not (pd->entries[dirIndex] & 1)) {
            pt = (PageTable*)kalloc4K();
            kmemset(pt,0,sizeof(PageTable));
            pd->entries[dirIndex] = ((uint32_t)pt) | 3;
        } else {
            pt = (PageTable*)(pd->entries[dirIndex] & 0xFFFFF000);
        }
        pt->entries[tblIndex] = (phys & 0xFFFFF000) | 3;
        invalidatePage(phys);
    }

    // Map an arbitrary physical region into the kernel page tables
    void mapIdentityRange(PageDirectory* pd, uintptr_t physStart, size_t length) {
        uintptr_t startPage = physStart & ~0xFFF;              // round down to 4K
        uintptr_t endPage = (physStart + length + 0xFFF) & ~0xFFF; // round up to 4K

        for (uintptr_t page = startPage; page < endPage; page += 0x1000) {
            mapIdentityPage(pd, page);
        }
    }

    bool updateDirectory(uint32_t phys_addr) {
        __asm__ __volatile__ (
            "mov %0, %%cr3"
            :
            : "r"(phys_addr)
            : "memory"
        );
        return true;
    }

    bool activatePaging() {
        asm volatile (
            "mov %%cr0, %%eax\n\t"
            "or $0x80000001, %%eax\n\t"
            "mov %%eax, %%cr0"
            :
            :
            : "eax", "memory"
        );
        enabled = true;
        return true;
    }

    int init(PageDirectory* directory) {
        updateDirectory((uint32_t)directory);
        if (enabled) {
            return 1;
            // more of a warning; cannot reactivate paging, but
            // the directory was updated
        }
        activatePaging();
        return 0;
    }

    // THIS CODE IS DIRECTLY COPIED FROM CHATGPT.
    // I JUST DELETED SO MUCH CODE THAT DID BASICALLY THIS AND
    // I DO NOT WANT TO EVER HAVE TO WRITE THAT AGAIN SOB
    PageDirectory* initwkp() {
        // 4 page tables for the kernel
        static PageTable kernelPTs[PAGE_TABLE_COUNT] alignas(4096); // 4 x 4MiB = 16MiB

        // Clear page directory
        for (int i = 0; i < 1024; i++) {
            kernelPD.entries[i] = 0;
        }

        // Fill page tables with identity mapping
        for (int pageTable = 0; pageTable < PAGE_TABLE_COUNT; pageTable++) {
            for (int pageNumber = 0; pageNumber < 1024; pageNumber++) {
                kernelPTs[pageTable].entries[pageNumber] = (pageNumber + pageTable*1024) * 0x1000 | 3; // Present + RW
            }
            // Point PD entry to this PT
            kernelPD.entries[pageTable] = ((uint32_t)&kernelPTs[pageTable]) | 3; // Present + RW
        }

        // Activate paging
        Paging::init(&kernelPD);

        return &kernelPD;
    }

    // (code is also directly from ChatGPT; i haven't learned G++ inline assembly syntax yet; very weird lookin.)
    bool is_paging_enabled() {
        uint32_t cr0;
        __asm__ __volatile__ (
            "mov %%cr0, %0"
            : "=r"(cr0)
        );
        return (cr0 & 0x80000000) != 0; // PG flag
    }
}