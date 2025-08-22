#include <stdint.h>
#include <string.h>
#include "types.h"
#include "memory.h"
#include "string.h"
#define EBDA_PTR 0x40E
#define RSDP_SIG "RSD PTR "
#define BIOS_START 0xE0000
#define BIOS_END 0x100000
#define ALIGNMENT 16

char* stratus; // pointer if anything demands a pointer

bool verify_rsdp_checksum(RSDPLegacyDescriptor* rsdp) {
    uint8_t sum = 0;
    uint8_t* bytes = (uint8_t*)rsdp;

    // base checksum (first 20 bytes always present)
    for (size_t i = 0; i < 20; i++) {
        sum += bytes[i];
    }
    if (sum != 0) return false;

    // if revision >= 2, validate the extended fields (36 bytes total)
    if (rsdp->revision >= 2) {
        sum = 0;
        for (size_t i = 0; i < 36; i++) {
            sum += bytes[i];
        }
        if (sum != 0) return false;
    }

    return true;
}

RSDPLegacyDescriptor* find_rsdp() {
	uint16_t EBDA = *(uint16_t*)EBDA_PTR;
	uint32_t EBDA_TRUE = (uint32_t)EBDA << 4;
	uint8_t* search = (uint8_t*)EBDA_TRUE;
	uint8_t* searchEnd = search+1024;

	for (; search < searchEnd; search+=ALIGNMENT) {
		if (kmemcmp(search,RSDP_SIG,8) == 0) {
			if (verify_rsdp_checksum((RSDPLegacyDescriptor*)search)) {
				return (RSDPLegacyDescriptor*)search;
			}
		}
	}

	search = (uint8_t*)BIOS_START;
	searchEnd = (uint8_t*)BIOS_END;

	for (; search < searchEnd; search+=ALIGNMENT) {
		if (kmemcmp(search,RSDP_SIG,8) == 0) {
			return (RSDPLegacyDescriptor*)search;
		}
	}

	return NULL; // Not found
}

Pair<uint8_t,void*> validate_rsdp() {
	RSDPLegacyDescriptor* rsdp = find_rsdp();
	if (rsdp == NULL) return {0x00,nullptr};
	if (rsdp->revision == 0) {
		return {rsdp->revision,rsdp};
	} else if (rsdp->revision == 2) {
		return {rsdp->revision,(RSDPModernDescriptor*)rsdp};
	}
	return {0x01,nullptr};
}

ACPITables find_rsdt() {
	Pair<uint8_t,void*> rsdp = validate_rsdp();

	ACPITables table;
	table.isValid = true;
	if (rsdp.second == nullptr) {
		table.isValid = false;
		table.count = rsdp.first;
		return table;
	}

	if (rsdp.first == 0) {
		// assert((((RSDPLegacyDescriptor*)(rsdp.second))->rsdt_address) != 0);
		// Terminal::print(parseInt((int)rsdp.second,stratus,16));
		uint32_t phys = ((RSDPLegacyDescriptor*)rsdp.second)->rsdt_address;
		RSDTDescriptor* rsdt = nullptr;

		// Early protected mode without paging, assuming A20 enabled:
		rsdt = (RSDTDescriptor*)(uintptr_t)phys;
		table.header = rsdt->header;
		size_t count = (table.header.length-sizeof(ACPISTDHeader))/sizeof(uint32_t);
		table.count = count;
		table.isXsdt = false;
		Terminal::print(parseInt(table.header.length,stratus,10));
		table.entries = (uint64_t*)Allocator::malloc(count*sizeof(uint64_t));
		// if (getError() == -102) {
		// 	fault(-102,"While finding RSDT in ACPI.");
		// }
		for (size_t i = 0; i < count; i++) {
			table.entries[i] = (uint64_t)rsdt->other_std[i];
		}
		// table.entries = ...whatever it is
	} else {
		XSDTDescriptor* xsdt = (XSDTDescriptor*)((RSDPModernDescriptor*)(rsdp.second))->xsdt_address;
		table.header = xsdt->header;
		size_t count = (table.header.length-sizeof(ACPISTDHeader))/sizeof(uint64_t);
		table.count = count;
		table.isXsdt = true;
		table.entries = xsdt->other_std;
	}
	return table;
}

void* find_sdt(ACPITables* tables, const char* signature) {
	for (size_t i = 0; i < tables->count; i++) {
		if (kmemcmp((void*)tables->entries[i],signature,4) == 0) {
			return (void*)tables->entries[i];
		}
	}
	return nullptr;
}