#include "drivers/framebuf.h"
#include "memory.h"
#include "serial.h"
#include "const.h"
#include "font.h"
namespace Terminal {
    uint32_t chrWidth = 0;
    uint32_t chrHeight = 0;
    uint32_t bufWidth = 0;
    uint32_t bufHeight = 0;
    uint8_t* table;
    uint32_t tableSize;
    uint32_t cursorX = 0;
    uint32_t cursorY = 0;
    // TODO: implement cursor (made easier because i store state)
    void enableCursor() {}
    void disableCursor() {}
    void updateCursorPos() {}
    void setCursorConfig(uint8_t config) {}
    void init(void) {
        bufWidth = Framebuffer::getWidth();
        bufHeight = Framebuffer::getHeight();
        chrWidth = bufWidth/Font::getChWidth();
        chrHeight = bufHeight/Font::getChHeight();
        tableSize = chrWidth*chrHeight;
        table = (uint8_t*)Allocator::kalloc(tableSize*sizeof(uint8_t));
    }
    uint32_t getWidth() {
        return chrWidth;
    }
    uint32_t getHeight() {
        return chrHeight;
    }
    size_t getIndex(size_t x, size_t y) {
        return y * chrWidth + x;
    }
    size_t getX(size_t index) {
        return index % chrWidth;
    }
    size_t getY(size_t index) {
        return index / chrWidth;
    }
    void drawWholeScreen(void) {
        for (size_t i = 0;i < tableSize;i++) {
            size_t x = getX(i);
            size_t y = getY(i);
            char ch = table[i];
            if (ch == '\0') ch = ' ';
            Font::blitCh(x,y,ch);
        }
    }
    void clearScreen(void) {
        cursorX = 0;
        cursorY = 0;
        kmemset(table, 0, tableSize);
        drawWholeScreen(); // this can be optimized with a specialized Framebuffer:clearScreen() call. unfortunately, i'm lazy.
    }
    void shiftUp(size_t rows) {
        if (rows >= chrHeight) {
            clearScreen();
            return;
        }
        size_t amount = chrWidth*rows;
        kmemcpy(table, table + amount, tableSize - amount);
        kmemset(table+tableSize-amount, 0, amount);
        drawWholeScreen();
    }
    void writeCh(const char ch,size_t x,size_t y) {
        table[getIndex(x,y)] = ch;
        Font::blitCh(x,y,ch);
    }
    void adjustCursor() {
        if (cursorX >= chrWidth) {
            cursorX = 0;
            cursorY++;
        }
        if (cursorY >= chrHeight) {
            shiftUp((cursorY+1)-chrHeight);
            cursorY = chrHeight-1;
        }
    }
    void putCh(const char ch) {
        adjustCursor();
        if (ch == '\n') {
            cursorX = 0;
            cursorY++;
        } else {
            writeCh(ch,cursorX,cursorY);
            cursorX++;
        }
        adjustCursor();
    }
    void print(const char* str) {
        for (size_t i = 0;str[i] != 0;i++) {
            putCh(str[i]);
        }
        Serial::print(str);
    }
    void printdebug(const char* str) {
        #ifdef CONST_DEBUGGING
        print(str);
        #endif
    }
    void setRow(int value) {
        cursorY = value;
    }
    void setCol(int value) {
        cursorX = value;
    }
    uint32_t getRow() {
        return cursorY;
    }
    uint32_t getCol() {
        return cursorX;
    }
}