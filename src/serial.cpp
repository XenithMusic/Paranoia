#include "utils.h"
#include "const.h"
#define COM1 0x3f8

namespace Serial {
    void init() {
        #ifdef CONST_DEBUGGING
        outb(COM1+1,0x00);
        outb(COM1+3,0x80);
        outb(COM1+0,0x03);
        outb(COM1+1,0x00);
        outb(COM1+3,0x03);
        outb(COM1+2,0xC7);
        outb(COM1+4,0x0B);
        outb(COM1+4,0x1E);
        outb(COM1+0,0xAE);
        if (inb(COM1 + 0) != 0xAe) {
            return;
        }
        outb(COM1+4,0x0F);
        return;
        #endif
    }
    bool is_transmit_empty() {
        #ifdef CONST_DEBUGGING
        return inb(COM1+5) & 0x20;
        #else
        return false;
        #endif
    }
    void write(char c) {
        #ifdef CONST_DEBUGGING
        if (c == '\n') {
            write('\r');
        }
        while (is_transmit_empty() == 0);
        outb(COM1,c);
        #endif
    }
    void print(char* c) {
        size_t i = 0;
        while (c[i] != 0) {
            write(c[i]);
            i++;
        }
    }
}