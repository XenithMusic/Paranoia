#include "types.h"
#include "string.h"
#include "serial.h"
#include "drivers/framebuf.h"
namespace Framebuffer {
    static framebuf_t framebuf;
    static bool initialized = false;
    framebuf_t init(multiboot_info_t* mbi) {
        framebuf = {
            true,
            (uint8_t*)mbi->framebuffer_addr,
            mbi->framebuffer_width,
            mbi->framebuffer_height,
            mbi->framebuffer_pitch,
            mbi->framebuffer_bpp,
            (FrameBufferType)mbi->framebuffer_type
        };
        if (framebuf.bpp != 32) return {};
        initialized = true;
        return framebuf;
    }
    void vaddr(uint32_t virt) {
        framebuf.address = (uint8_t*)virt;
        Serial::print("FB virtual: 0x");
            Serial::print(parseU32((uint32_t)framebuf.address,16));
            Serial::print("\n");
        Serial::print("FB pitch: ");
            Serial::print(parseU32((uint32_t)framebuf.pitch,10));
            Serial::print("\n");
        Serial::print("FB dimensions: ");
            Serial::print(parseU32((uint32_t)framebuf.width,10));
            Serial::print(",");
            Serial::print(parseU32((uint32_t)framebuf.height,10));
            Serial::print("\n");
    }
    void drawpx(uint32_t x, uint32_t y, uint32_t col) {
        if (!initialized) return;
        if (x >= framebuf.width or y >= framebuf.height) {
            return;
        }
        uint32_t offset = y * framebuf.pitch + x * (framebuf.bpp>>3);
        uint32_t* volatile ptr = (uint32_t*)(framebuf.address + offset);
        *ptr = col;
    }
}