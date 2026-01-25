#include "types.h"

RSDPLegacyDescriptor* find_rsdp();

Pair<uint8_t,RSDPLegacyDescriptor*> validate_rsdp();

ACPITables find_rsdt();

void* find_sdt(ACPITables* tables, const char* signature);