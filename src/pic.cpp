#include "utils.h"


namespace pic
{
    uint8_t in(bool isData) {
        uint16_t port = 0x20;
        if (isData) port = 0x21;
        return inb(port);
    }
    
    void out(bool isData, uint8_t data) {
        uint16_t port = 0x20;
        if (isData) port = 0x21;
        outb(port,data);
    }
    
    uint8_t inblocking(bool isData, int timeout = 0) {
        if (timeout == -1) {
            while ((in(false) & 0x01) == 0);
        } else {
            while ((in(false) & 0x01) == 0 and timeout >= 0) timeout--;
        }
        return in(isData);
    }
    
    bool outack(bool isData, uint8_t data) {
        out(isData,data);
        uint8_t ack = inblocking(true);
        return ack == 0xFA;
    }
    
    void mask(uint8_t ands,uint8_t ors) {
        uint8_t mask = in(true);
        mask &= ands;
        mask |= ors;
        out(true,mask);
    }
}