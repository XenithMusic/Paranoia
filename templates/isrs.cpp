#include <stdint.h>
#include "idt.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexcessive-regsave"

#define ISR(n) \
    __attribute__((interrupt)) \
    void isr_##n(InterruptFrame* frame) { \
        alignas(16) ubyte_t fpuState[512]; \
        asm volatile("fxsave %0" : "=m" (fpuState)); \
        IDT::dispatch(n,frame,0); \
        asm volatile("fxrstor %0" : : "m" (fpuState)); \
    }
#define ISR_E(n) \
    __attribute__((interrupt)) \
    void isr_##n(InterruptFrame* frame,uint32_t errorCode) { \
        alignas(16) ubyte_t fpuState[512]; \
        asm volatile("fxsave %0" : "=m" (fpuState)); \
        IDT::dispatch(n,frame,errorCode); \
        asm volatile("fxrstor %0" : : "m" (fpuState)); \
    }

/*$$"\n".join([(f"ISR_E({x})" if x in [8,10,11,12,13,14,17,21,29,30] else f"ISR({x})") for x in range(256)])#*/
extern "C" uintptr_t isr_stub_table[256] = {
/*$$"\n".join([f"(uintptr_t)&isr_{x}," for x in range(256)])#*/
};

#pragma clang diagnostic pop