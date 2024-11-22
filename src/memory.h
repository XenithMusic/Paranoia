#include "types.h"

extern "C" {
	namespace Allocator {

	    enum MMAPAccessType {
	    	ALLOCATE, // Find a location to allocate memory (target = how much)
	    	FREE      // Find a location to free memory     (target = location)
	    };

		void getMatchingMMAP(multiboot_info* mbi, enum MMAPAccessType type, uint32_t target);

		void* kmalloc(size_t size);
	}
}