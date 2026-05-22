#include "types.h"
#include "utils.h"

#define REGISTER_PORT 0x70
#define VALUE_PORT 0x71

namespace CMOS {
    bool NMI_disable = true;
    void delay() {
        for (int i=0;i<10;i++) {}
    }
    void set_register(ubyte1_t reg) {
        outb(REGISTER_PORT, (NMI_disable << 7) | reg);
        delay();
    }
    bool setNMIdisable(bool state) {
        NMI_disable = state;
        set_register(0xD);
        return state;
    }
    ubyte1_t read_register(ubyte1_t reg) {
        set_register(reg);
        return inb(0x71);
    }
    void write_register(ubyte1_t reg,ubyte1_t value) {
        set_register(reg);
        outb(0x71,value);
    }
}