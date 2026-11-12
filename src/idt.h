#include "memory.h"
#include <stdint.h>
#include "isrs.h"
#include "types.h"
#include "utils.h"

#define IDT_ENTRIES 256
#define IDT_TABLE_SIZE sizeof(void*)*IDT_ENTRIES
#define IDT_ENTRIES_SIZE sizeof(IDTEntry)*IDT_ENTRIES

namespace IDT {
    void genericFaultHandler(int index,InterruptFrame* frame, uint32_t errorCode);
    void harshFaultHandler(int index,InterruptFrame* frame, uint32_t errorCode);
    void breakpointHandler(int index,InterruptFrame* frame, uint32_t errorCode);
    void init();
    void add(uint8_t index, void* handler);
    void addDefaults();
    void finish();
    void dispatch(int index, InterruptFrame* frame, uint32_t errorCode);
    void handleQueue();
    void setUserspace(int index, bool state);
    void setDeferred(int index, bool state);
}