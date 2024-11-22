#include "types.h"

extern "C" {
	namespace Allocator {

		void init(multiboot_info* mbi);

		void getMatchingMMAP(multiboot_info* mbi, enum MMAPAccessType type, uint32_t target);

		void* kmalloc(size_t size);
	}
}