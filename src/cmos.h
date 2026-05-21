#include "types.h"

namespace CMOS {
    bool setNMIdisable(bool state);
    ubyte1_t read_register(ubyte1_t reg);
    ubyte1_t write_register(ubyte1_t reg,ubyte1_t value);
}