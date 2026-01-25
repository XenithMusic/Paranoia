#include "types.h"

/*

Copyright (C) 2026  XenithMusic (on github)

The Paranoia kernel is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Paranoia is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Paranoia. If not, see <https://www.gnu.org/licenses/>.

*/

namespace Paging {
    // the basic page directory for the kernel
    static PageDirectory kernelPD alignas(4096);

    // allocates 12K, and rounds the pointer up to the next 4K boundary.
    void* kalloc4K();

    void invalidatePage(uint32_t phys);
    bool updateDirectory(uint32_t phys_addr);
    bool activatePaging();

    // Initializes paging with a specified PageDirectory.
    // This will update the paging directory if already initialized.
    int init(PageDirectory* directory);
    // (code is from ChatGPT because i didn't wanna rewrite all of this.)
    //
    // Calls `Paging::init()` with 4 identity mapped tables covering 16MiB. Intended for kernel use only.
    PageDirectory* initwkp();
    
    // Maps an identity page at a physical address.
    void mapIdentityPage(PageDirectory* pd, uint32_t phys);
    void mapIdentityRange(PageDirectory* pd, uintptr_t physStart, size_t length);
    bool is_paging_enabled();
}