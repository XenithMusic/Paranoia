#include <stdint.h>
#include <stddef.h>
#include "types.h"

/*

Copyright (C) 2024  XenithMusic (on github)

The Paranoia kernel is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Paranoia is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Paranoia. If not, see <https://www.gnu.org/licenses/>.

*/


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