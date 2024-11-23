#include "types.h"
#include "stddef.h"
#include "terminal.h"
#include "string.h"

namespace Allocator {

    multiboot_info* mbi;
    char* tempstr;

    void init(multiboot_info* mbi) {
    	mbi = mbi;
    }
}