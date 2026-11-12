#include <stdint.h>
#include "idt.h"

// TODO: add `-mgeneral-regs-only` when compiling isrs

#define ISR(n) \
    __attribute__((interrupt)) \
    void isr_##n(InterruptFrame* frame) { \
        IDT::dispatch(n,frame,0); \
    }
#define ISR_E(n) \
    __attribute__((interrupt)) \
    void isr_##n(InterruptFrame* frame,uint32_t errorCode) { \
        IDT::dispatch(n,frame,errorCode); \
    }

/*$$"\n".join([(f"ISR_E({x})" if x in [8,10,11,12,13,14,17,21,29,30] else f"ISR({x})") for x in range(256)])#*/
extern "C" uintptr_t isr_stub_table[256] = {
/*$$"\n".join([f"&isr_{x}" for x in range(256)])#*/
};