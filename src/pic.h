#include "stdint.h"

namespace pic
{
    void remapPIC(uint8_t PIC1_IRQ_MASK,uint8_t PIC2_IRQ_MASK);
    void mask(uint8_t ands,uint8_t ors);
} // namespace pic
