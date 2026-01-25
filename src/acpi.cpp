#include <stdint.h>
#include <string.h>
#include "types.h"
#include "memory.h"
#include "string.h"
#include "page.h"

/*

Copyright (C) 2026  XenithMusic (on github)

The Paranoia kernel is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Paranoia is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Paranoia. If not, see <https://www.gnu.org/licenses/>.

*/

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

void* find_signature(const void* signature, uint8_t* search, uint8_t* searchEnd) {
	for (; search < searchEnd; search+=ALIGNMENT) {
		if (kmemcmp(search,signature,8) == 0) {
			if (verify_rsdp_checksum((RSDPLegacyDescriptor*)search)) {
				// Terminal::print("Funden.\n");
				return (RSDPLegacyDescriptor*)search;
			}
		}
	}
	return NULL;
}

RSDPLegacyDescriptor* find_rsdp() {
	uint16_t EBDA = *(volatile uint16_t*)EBDA_PTR;
	uint32_t EBDA_TRUE = (uint32_t)EBDA << 4;
	uint8_t* search = (uint8_t*)EBDA_TRUE;
	uint8_t* searchEnd = search+1024;

	void* returned = find_signature(RSDP_SIG,search,searchEnd);

	if (returned != NULL)

		if (verify_rsdp_checksum((RSDPLegacyDescriptor*)returned)) {
			return (RSDPLegacyDescriptor*)returned;
		}

	// for (; search < searchEnd; search+=ALIGNMENT) {
	// 	if (kmemcmp(search,RSDP_SIG,8) == 0) {
	// 		if (verify_rsdp_checksum((RSDPLegacyDescriptor*)search)) {
	// 			// Terminal::print("Funden.\n");
	// 			return (RSDPLegacyDescriptor*)search;
	// 		}
	// 	}
	// }

	search = (uint8_t*)BIOS_START;
	searchEnd = (uint8_t*)BIOS_END;

	// for (; search < searchEnd; search+=ALIGNMENT) {
	// 	if (kmemcmp(search,RSDP_SIG,8) == 0) {
	// 		return (RSDPLegacyDescriptor*)search;
	// 	}
	// }

	returned = find_signature(RSDP_SIG,search,searchEnd);

	if (returned != NULL)

		if (verify_rsdp_checksum((RSDPLegacyDescriptor*)returned)) {
			return (RSDPLegacyDescriptor*)returned;
		}

	return NULL; // Not found
}

// Pair.first is the revision.
// if (Pair.first == 0) {
//		Pair.second is a pointer to a RSDPLegacyDescriptor
// } else {
// 		Pair.second is a pointer to a RSDPModernDescriptor
// }
Pair<uint8_t,RSDPLegacyDescriptor*> validate_rsdp() {
	RSDPLegacyDescriptor* rsdp = find_rsdp();
	if (rsdp == NULL) return {0x00,nullptr};
	return {rsdp->revision,rsdp}; // expected to cast manually.
	// if (rsdp->revision >= 0) {
	// 	return {rsdp->revision,rsdp};
	// } else {
	// 	return {rsdp->revision,rsdp};
	// }
	// return {0x01,nullptr};
}

ACPITables find_rsdt() {
	Pair<uint8_t,RSDPLegacyDescriptor*> rsdp = validate_rsdp();

	ACPITables table;
	table.isValid = true;
	if (rsdp.second == nullptr) {
		table.isValid = false;
		table.count = rsdp.first;
		return table;
	}

	// Terminal::print(parseInt(rsdp.second->checksum,stratus,16));

	if (rsdp.first >= 2) {
		XSDTDescriptor* xsdt = (XSDTDescriptor*)((RSDPModernDescriptor*)(rsdp.second))->xsdt_address;
		table.header = xsdt->header;
		size_t count = (table.header.length-sizeof(ACPISTDHeader))/sizeof(uint64_t);
		table.count = count;
		table.isXsdt = true;
		table.entries = xsdt->other_std;
	} else {
		RSDPLegacyDescriptor* descriptor = (RSDPLegacyDescriptor*)rsdp.second;
		// assert((((RSDPLegacyDescriptor*)(rsdp.second))->rsdt_address) != 0);
		// Terminal::print(parseInt((int)rsdp.second,stratus,16));
		uint32_t phys = descriptor->rsdt_address;
		// Paging::mapIdentityRange(&Paging::kernelPD,phys,sizeof(RSDTDescriptor));
		RSDTDescriptor* rsdt = nullptr;

		// Early protected mode without paging, assuming A20 enabled:
		rsdt = (RSDTDescriptor*)(uintptr_t)phys;
		Terminal::print(parseInt(phys,stratus,16));
		return {};
		table.header = rsdt->header;
		size_t count = (table.header.length-sizeof(ACPISTDHeader))/sizeof(uint32_t);
		table.count = count;
		table.isXsdt = false;
		// char* sig = descriptor->signature;
		// sig[7] = '\0';
		// Terminal::print(sig);
		// Terminal::print("\n\n\nMaking sure it SHOULD show something");
		table.entries = (uint64_t*)Allocator::kalloc(count*sizeof(uint64_t));
		// if (getError() == -102) {
		// 	fault(-102,"While finding RSDT in ACPI.");
		// }
		for (size_t i = 0; i < count; i++) {
			table.entries[i] = (uint64_t)rsdt->other_std[i];
		}
		// table.entries = ...whatever it is
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