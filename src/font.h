#include <stddef.h>
#include "types.h"

namespace Font {
    size_t getChWidth();
    size_t getChHeight();
    void init();
    void loadFallbackFont();
    void loadFont(VFS::File fontfile);
    uint32_t getPxOfCh(size_t x, size_t y, char sch);
    void blitCh(uint32_t x, uint32_t y, char sch);
    void print(uint32_t x, uint32_t y, const char* str);
}