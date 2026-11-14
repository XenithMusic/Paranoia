#include "types.h"
#include <stdint.h>
typedef union {
    int paletteIdx;
    int r;
    int g;
    int b;
} pixelColor;
enum FrameBufferType {
    INDEXED=0,
    RGB=1,
    EGA=2
};
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
    framebuf_t init(multiboot_info_t* mbi);
    void vaddr(uint32_t virt);
    void drawpx(uint32_t x, uint32_t y, uint32_t col);
}