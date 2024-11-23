#include "stdint.h"

#pragma once

enum MMAPAllocationType : uint32_t {
	ALLOCATED = 0x00,
	AVAILABLE = 0x01,
	RESERVED = 0x02
};

typedef struct {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t mmap_length;
    uint32_t mmap_addr;
} multiboot_info;

typedef struct {
    uint32_t size; // size of the `mmap` entry
    uint32_t addr; // start of the memory region
    uint32_t len;  // length of the entry (`size` is for the entry)
    enum MMAPAllocationType type; // type of memory
} mmap_entry;

enum MMAPAccessType {
	ALLOCATE, // Find a location to allocate memory (target = how much)
	FREE      // Find a location to free memory     (target = location)
};