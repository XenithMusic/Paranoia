#include "types.h"
#include <stdint.h>
typedef union {
    int paletteIdx;
    int r;
    int g;
    int b;
} pixelColor;
struct framebuf_t {
    bool valid = false;
    uint8_t* address;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint8_t bpp;
    FrameBufferType type;
};
namespace Framebuffer {
    uint32_t getWidth();
    uint32_t getHeight();
    framebuf_t init(Multiboot2::FramebufferTag* fbtag);
    void vaddr(uint32_t virt);
    uint32_t* getpxaddr(uint32_t x, uint32_t y);
    void drawpx(uint32_t x, uint32_t y, uint32_t col);
}