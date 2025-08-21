#include <stdint.h>
#include <string.h>
#include "types.h"
#include "memory.h"
#define EBDA_PTR 0x40E
#define RSDP_SIG "RSD PTR "
#define BIOS_START 0xE0000
#define BIOS_END 0x100000
#define ALIGNMENT 16

RSDPLegacyDescriptor* find_rsdp() {
	uint16_t EBDA = *(uint16_t*)EBDA_PTR;
	uint32_t EBDA_TRUE = (uint32_t)EBDA << 4;
	uint8_t* search = (uint8_t*)EBDA_TRUE;
	uint8_t* searchEnd = search+1024;

	for (; search < searchEnd; search+=ALIGNMENT) {
		if (kmemcmp(search,RSDP_SIG,8)) {
			return (RSDPLegacyDescriptor*)search;
		}
	}

	search = (uint8_t*)BIOS_START;
	searchEnd = (uint8_t*)BIOS_END;

	for (; search < searchEnd; search+=ALIGNMENT) {
		if (kmemcmp(search,RSDP_SIG,8)) {
			return (RSDPLegacyDescriptor*)search;
		}
	}

	return NULL; // Not found
}

Pair<uint8_t,void*> validate_rsdp() {
	RSDPLegacyDescriptor* rsdp = find_rsdp();
	if (rsdp->revision == 0) {
		return {rsdp->revision,rsdp};
	} else if (rsdp->revision == 2) {
		return {rsdp->revision,(RSDPModernDescriptor*)rsdp};
	}
}