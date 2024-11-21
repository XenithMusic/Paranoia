#include <stdint.h>
#include <stddef.h>
#include "string.h"

extern "C" {

    namespace Terminal {
        enum vga_color {
            VGA_COLOR_BLACK = 0,
            VGA_COLOR_BLUE = 1,
            VGA_COLOR_GREEN = 2,
            VGA_COLOR_CYAN = 3,
            VGA_COLOR_RED = 4,
            VGA_COLOR_MAGENTA = 5,
            VGA_COLOR_BROWN = 6,
            VGA_COLOR_LIGHT_GREY = 7,
            VGA_COLOR_DARK_GREY = 8,
            VGA_COLOR_LIGHT_BLUE = 9,
            VGA_COLOR_LIGHT_GREEN = 10,
            VGA_COLOR_LIGHT_CYAN = 11,
            VGA_COLOR_LIGHT_RED = 12,
            VGA_COLOR_LIGHT_MAGENTA = 13,
            VGA_COLOR_LIGHT_BROWN = 14,
            VGA_COLOR_WHITE = 15,
        };

        static const size_t VGA_WIDTH = 80;
        static const size_t VGA_HEIGHT = 25;

        static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) 
        {
            return fg | bg << 4;
        }

        static inline uint16_t vga_entry(unsigned char uc, uint8_t color) 
        {
            return (uint16_t) uc | (uint16_t) color << 8;
        }

        size_t row;
        size_t col;
        uint8_t color;
        uint16_t* buffer;
        uint16_t* clearBuffer;
        uint16_t* temp;

        uint8_t getTerminalColor() { return color; }

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

        void init(void)
        {
            row = 0;
            col = 0;
            color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
            buffer = (uint16_t*) 0xB8000;
            clearBuffer = (uint16_t*) 0xC0000;
            clearScreen();
        }

        void putEntryAt(char c, uint8_t color, size_t x, size_t y) {
            const size_t index = y * VGA_WIDTH + x;
            buffer[index] = vga_entry(c,color);
        }

        void putChar(char c) {
            if (c == '\n') {
                col = 0;
                if (++row == VGA_HEIGHT) {
                    scrollUp(1);
                }
                return;
            }
            putEntryAt(c, color, col, row);
            if (++col >= VGA_WIDTH) {
                col = 0;
                if (++row == VGA_HEIGHT) {
                    scrollUp(1);
                }
            }
        }
        void write(const char* c,size_t size) {
            for (size_t i = 0; i < size; i++) {
                putChar(c[i]);
            }
        }
        void print(const char* str) {
            write(str,strlen(str));
        }
    }

};