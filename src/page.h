#include "types.h"

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