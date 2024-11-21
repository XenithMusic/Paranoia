#include <stdint.h>
#include <stddef.h>

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

        static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg);

        static inline uint16_t vga_entry(unsigned char uc, uint8_t color);

        uint8_t getTerminalColor();

        void init(void);

        void swapBuffers(void);

        void putEntryAt(char c, uint8_t color, size_t x, size_t y);

        void putChar(char c);
        void write(const char* c,size_t size);
        void print(const char* str);
    }
};