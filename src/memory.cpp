#include "types.h"

extern "C" {
	namespace Allocator {

	    multiboot_info* mbi;

	    void init(multiboot_info* mbi) {
	    	mbi = mbi;
	    }

		mmap_entry* getMatchingMMAP(multiboot_info* mbi, enum MMAPAccessType type, uint32_t target) {
			mmap_entry* mmap = (mmap_entry*)mbi->mmap_addr;
			uint32_t mmap_length = mbi->mmap_length;

			while ((uint32_t)mmap < mbi->mmap_addr + mmap_length) {
				if (type == ALLOCATE and mmap->type == FREE and mmap->len >= target) {
					return mmap;
				}
				if (type == FREE and mmap->type == ALLOCATED and mmap->addr == target) {
					return mmap;
				}
				mmap = (mmap_entry*)((uint32_t)mmap + mmap->size + sizeof(mmap->size));
			}
			return nullptr;
		}

		void* kmalloc(size_t size) {
			mmap_entry* allocatedBlock = getMatchingMMAP(mbi, ALLOCATE, size);
			if (allocatedBlock == nullptr) {
				return nullptr;
			}
			// TODO: Split allocatedBlock if it's larger than requested.
			allocatedBlock->type = ALLOCATED;
			return reinterpret_cast<void*>(allocatedBlock->addr);
		}

		// TODO: Make a freeing function `void kfree(void* pointer);`
	}
}