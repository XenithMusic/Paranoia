#include "types.h"
namespace Multiboot2 {
    void* getTagOfType(multiboot2_info_t* info,Multiboot2TagType type);
    RSDPLegacyDescriptor* getLegacyRSDP(multiboot2_info_t* info);
    RSDPModernDescriptor* getModernRSDP(multiboot2_info_t* info);
    FramebufferTag* getFramebuf(multiboot2_info_t* info);
}