#include <stddef.h>
#include "memory.h"
#include "types.h"

template <typename T>
struct CircQueue {
    T* volatile array = nullptr;
    size_t size = 0;
    int readIndex = 0;
    volatile int writeIndex = 0;
    void init(size_t requestedSize) {
        if (size != 0) {
            if (size != requestedSize) {
                fault(1,"CircQueue redefinition with differing size.");
            } else {
                warn("CircQueue redefinition.\n");
            }
        }
        size = requestedSize;
        array = (IDTDispatchArgs*)Allocator::kalloc(size*sizeof(T));
    }
    Optional<T> read() {
        Optional<T> response;
        if (readIndex == writeIndex) {
            return response;
        }
        T val = array[readIndex];
        readIndex++;
        response.present = true;
        response.value = val;
        return response;
    }
    void write(T val) {
        if (writeIndex == readIndex-1) {
            warn("CircQueue is full!");
            return;
        }
        array[writeIndex] = val;
        writeIndex++;
    }
    size_t elements() {
        size_t elements = writeIndex-readIndex;
        if (elements < 0) elements += size;
        return elements;
    }
};