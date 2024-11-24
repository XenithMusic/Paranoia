#include <stdint.h>
#include <stddef.h>
#include "types.h"

namespace Terminal {

    static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg);

    static inline uint16_t vga_entry(unsigned char uc, uint8_t color);

    uint8_t getTerminalColor();

    void init(void);

    void swapBuffers(void);

    void clearScreen(void);

    void putEntryAt(char c, uint8_t color, size_t x, size_t y);

    void putChar(char c);
    void write(const char* c,size_t size);
    void print(const char* str);
    void setRow(int value);
    void setCol(int value);
}