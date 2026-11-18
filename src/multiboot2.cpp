#include "types.h"
namespace Multiboot2 {
    void* getTagOfType(multiboot2_info_t* info,Multiboot2TagType type) {
        if (info == nullptr) return nullptr;
        uint8_t* ptr = (uint8_t*)(info + 1);
        uint8_t* end = ((uint8_t*)(info) + info->total_size);
        while ((ptr + sizeof(multiboot2_tag_t)) <= end) {
            multiboot2_tag_t* tag = (multiboot2_tag_t*)(ptr);
            if (tag->type == type) {
                return tag;
            }
            if (tag->type == TAG_END) return nullptr;
            if (tag->size < 8) return nullptr;
            ptr += (tag->size + 7) & ~7;
        }
        return nullptr;
    }
    RSDPLegacyDescriptor* getLegacyRSDP(multiboot2_info_t* info) {
        RSDPv1Tag* tag = (RSDPv1Tag*)getTagOfType(info,TAG_RSDP_OLD);
        if (tag == nullptr) return nullptr;
        return &tag->rsdp;
    }
    RSDPModernDescriptor* getModernRSDP(multiboot2_info_t* info) {
        RSDPv2Tag* tag = (RSDPv2Tag*)getTagOfType(info,TAG_RSDP_NEW);
        if (tag == nullptr) return nullptr;
        return &tag->rsdp;
    }
}