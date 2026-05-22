#pragma once
#include "types.h"

namespace disk {
    bool get_bytes(uint16_t driveIndex, uint16_t partitionIndex, size_t startAddress,size_t bytes,void* destination);
    bool get_bytes_raw(uint16_t driveIndex, size_t startAddress,size_t bytes,void* destination);
    bool write_bytes_raw(uint16_t driveIndex,size_t startAddress,size_t bytes,void* source);
    bool write_bytes(uint16_t driveIndex, uint16_t partitionIndex, size_t startAddress,size_t bytes,void* source);
}