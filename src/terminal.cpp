#include <stdint.h>
#include <stddef.h>
#include "string.h"
#include "types.h"
#include "const.h"
#include "utils.h"

/*

Copyright (C) 2026  XenithMusic (on github)

The Paranoia kernel is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Paranoia is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Paranoia. If not, see <https://www.gnu.org/licenses/>.

*/


namespace Terminal {

    static const size_t VGA_WIDTH = 80;
    static const size_t VGA_HEIGHT = 25;

    uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) 
    {
        return fg | bg << 4;
    }

    static inline uint16_t vga_entry(unsigned char uc, uint8_t color) 
    {
        return (uint16_t) uc | (uint16_t) color << 8;
    }

    size_t row;
    size_t col;
    size_t crow;
    size_t ccol;
    uint8_t color;
    uint8_t cursorConfig = 0xEF;
    uint16_t* buffer;
    uint16_t* clearBuffer;
    uint16_t* temp;

    uint8_t getTerminalColor() { return color; }

    void setCursorConfig(uint8_t config) {
        cursorConfig = config;
    }

    void enableCursor() {
        outb(0x3D4, 0x0A);
        outb(0x3D5, (inb(0x3D5) & 0xC0) | ((cursorConfig >> 4) & 0xF));

        outb(0x3D4, 0x0B);
        outb(0x3D5, (inb(0x3D5) & 0xE0) | ((cursorConfig) & 0xF));
    }

    void disableCursor() {
        outb(0x3D4, 0x0A);
        outb(0x3D5, 0x20);
    }

    void updateCursorPos() {
        uint16_t pos = crow * VGA_WIDTH + ccol;
        outb(0x3D4, 0x0F);
        outb(0x3D5, (uint8_t) (pos & 0xFF));
        outb(0x3D4, 0x0E);
        outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));

    }

    void setCursorPos(int x, int y) {
        ccol = x;
        crow = y;
    }

    void scrollUp(size_t n) {
        // Ensure n is not larger than the number of rows
        if (n > VGA_HEIGHT) {
            n = VGA_HEIGHT;
        }

        // Shift all rows up by `n`
        for (size_t y = 0; y < VGA_HEIGHT - n; y++) {
            for (size_t x = 0; x < VGA_WIDTH; x++) {
                size_t index = (y + n) * VGA_WIDTH + x; // Calculate the source index
                size_t targetIndex = y * VGA_WIDTH + x; // Calculate the target index
                buffer[targetIndex] = buffer[index];    // Copy data up
            }
        }

        // Clear the last `n` rows at the bottom of the screen
        for (size_t y = VGA_HEIGHT - n; y < VGA_HEIGHT; y++) {
            for (size_t x = 0; x < VGA_WIDTH; x++) {
                size_t index = y * VGA_WIDTH + x;
                buffer[index] = vga_entry(' ', color);  // Clear with empty space
            }
        }
        row -= n;
    }

    void swapBuffers(void)
    {
        uint16_t* temp = buffer;
        buffer = clearBuffer;
        clearBuffer = temp;
    }

    void clearScreen(void)
    {
        swapBuffers();
        for (size_t y = 0; y < VGA_HEIGHT; y++) {
            for (size_t x = 0; x < VGA_WIDTH; x++) {
                const size_t index = y * VGA_WIDTH + x;
                clearBuffer[index] = vga_entry(' ', color);
            }
        }
        swapBuffers();
    }

    void GPL();

    void init(void)
    {
        row = 0;
        col = 0;
        color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        buffer = (uint16_t*) 0xB8000;
        clearBuffer = (uint16_t*) 0xC0000;
        clearScreen();
        GPL();
    }

    void putEntryAt(char c, uint8_t color, size_t x, size_t y) {
        const size_t index = y * VGA_WIDTH + x;
        buffer[index] = vga_entry(c,color);
    }

    void putChar(char c,bool moveCursor) {
        if (c == '\n') {
            col = 0;
            if (moveCursor) ccol = 0;
            if (moveCursor) crow++;
            if (++row == VGA_HEIGHT) {
                scrollUp(1);
                if (moveCursor) crow--;
            }
            updateCursorPos();
            return;
        }
        putEntryAt(c, color, col, row);
        if (moveCursor) ccol++;
        if (++col >= VGA_WIDTH) {
            if (moveCursor) ccol = 0;
            if (moveCursor) crow++;
            col = 0;
            if (++row == VGA_HEIGHT) {
                scrollUp(1);
                if (moveCursor) crow--;
            }
        }
        updateCursorPos();
    }
    void write(const char* c,size_t size,bool moveCursor) {
        for (size_t i = 0; i < size; i++) {
            putChar(c[i],moveCursor);
            if (c[i] == '\n') {
            }
        }
    }
    void print(const char* str,bool moveCursor) {
        write(str,strlen(str),moveCursor);
    }
    void print(const char* str) {
        print(str,true);
    }
    void printdebug(const char* str) {
        if (CONST_DEBUGGING)
            return print(str);
        return;
    }
    void GPL() {
        print("Paranoia  Copyright (C) 2024  XenithMusic\n");
        print("This program comes with ABSOLUTELY NO WARRANTY\n");
        print("This is free software, and you are welcome to redistribute it\n");
        print("under certain conditions.\n");
        print("See the GPLv3 license for more details.\n");
        print("\n");
    }

    void setRow(int value) {
        row = value;
    }

    void setCol(int value) {
        col = value;
    }
}
/*
    <program>  Copyright (C) <year>  <name of author>
    This program comes with ABSOLUTELY NO WARRANTY; for details type `show w'.
    This is free software, and you are welcome to redistribute it
    under certain conditions; type `show c' for details.
*/