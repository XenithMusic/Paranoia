#include <cstdint>
#include <string.h>
#include "types.h"
#define ebdaptr 0x40E
#define RSDP_SIG "RSD PTR "
#define BIOS_START 0xE0000
#define BIOS_END 0x100000
#define ALIGNMENT 16

RSDPDescriptor* find_rsdp() {
	// First, try EBDA
	uint16_t* ebda_seg_ptr = (uint16_t*)0x40E;
	uint32_t ebda_phys = (*ebda_seg_ptr) << 4;

	for (uint32_t addr = ebda_phys; addr < ebda_phys + 1024; addr += ALIGNMENT) {
		if (memcmp((char*)addr, RSDP_SIG, 8) == 0)
			return (RSDPDescriptor*)addr;
	}

	// Then, search BIOS memory area
	for (uint32_t addr = BIOS_START; addr < BIOS_END; addr += ALIGNMENT) {
		if (memcmp((char*)addr, RSDP_SIG, 8) == 0)
			return (RSDPDescriptor*)addr;
	}

	return NULL; // Not found
}

