#include <stdint.h>
namespace IDT {
    static void** handlerTable;
    static void** idtTable;
    void init();
    void add(uint8_t index, void* handler);
}