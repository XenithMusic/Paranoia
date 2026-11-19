#include <stddef.h>
#include <stdint.h>
#include "types.h"
#include "drivers/vfs.h"
#include "drivers/framebuf.h"
#include "data.h"
#include "serial.h"

namespace Font {
    const size_t w = 8; // character width
    const size_t h = 16; // character height
    const size_t cpr = 16; // columns per row
    fid_t font = 0;
    uint32_t* fontMem;
    bool fontLoaded = false;
    size_t getChWidth() {
        return w;
    }
    size_t getChHeight() {
        return h;
    }
    void init() {
    }
    void loadFallbackFont() {
        fontLoaded = true;
        fontMem = (uint32_t*)Allocator::kalloc(Data::fallbackFontSize);
        kmemcpy(fontMem, Data::fallbackFont, Data::fallbackFontSize);
    }
    void loadFont(VFS::File fontfile) {
        fontLoaded = true;
        font = VFS::openFile(fontfile);
        size_t size = VFS::getFileSize(font);
        fontMem = (uint32_t*)Allocator::kalloc(size);
        VFS::readFile(font, 0, size, fontMem);
    }
    uint32_t getPxOfCh(size_t x, size_t y, const uint8_t sch) {
        if (fontLoaded == false) loadFallbackFont();
        if (x >= w || y >= h) return -1; // no negative check as x,y is unsigned. -1 is MAX

        uint8_t ch = sch;
        size_t iw = cpr*w;
        size_t cx = ch % cpr;
        size_t cy = ch / cpr;
        size_t cax = cx*w;
        size_t cay = cy*h;
        size_t tx = cax+x;
        size_t ty = cay+y;
        size_t index = ty*iw + tx;

        return fontMem[index];
    }
    void blitCh(uint32_t x, uint32_t y, const uint8_t sch) {
        size_t ax = x*w;
        size_t ay = y*h;
        for (size_t cx=0;cx < w;cx++) {
            for (size_t cy=0;cy < h;cy++) {
                Framebuffer::drawpx(ax+cx, ay+cy, getPxOfCh(cx,cy,sch));
            }
        }
    }
    void print(uint32_t x, uint32_t y, const char* str) {
        size_t i = 0;
        while (str[i] != '\0') {
            blitCh(x+i, y, str[i]);
            i++;
        }
    }
}