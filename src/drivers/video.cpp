#include "types.h"

namespace video {
    Framebuffer* fb;
    void init(Framebuffer* framebuf) {
        fb = framebuf;
    }

    bool putpx(int x, int y, uint32_t color) {
        uintptr_t pixelAddr = fb->framebuffer_addr;
        pixelAddr += x*(fb->framebuffer_bpp/8);
        pixelAddr += y*fb->framebuffer_pitch;
        uint32_t* pixel = (uint32_t*)pixelAddr;
        *pixel = color;
        return *pixel == color;
    }
}