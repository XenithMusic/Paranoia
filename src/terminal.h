#include <stdint.h>

namespace Terminal {
    uint32_t getWidth();
    uint32_t getHeight();
    void enableCursor();
    void disableCursor();
    void updateCursorPos();
    void setCursorConfig(uint8_t config);
    void init(void);
    void clearScreen(void);
    void print(const char* str);
    void printdebug(const char* str);
    void setRow(int value);
    void setCol(int value);
    uint32_t getRow();
    uint32_t getCol();
}