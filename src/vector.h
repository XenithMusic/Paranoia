#pragma once
#include "memory.h"

#define MINIMUM_VECTOR_SIZE 256

template <typename T>
struct Vector {
    T* array = nullptr;
    size_t length; // items in the Vector
    size_t size; // maximum number of items the Vector can hold
    void resize() {
        if (array == nullptr) return;
        size_t targetSize = 0;
        if (size/4 > length && size > MINIMUM_VECTOR_SIZE) {
            targetSize = size/2;
        } else if (length >= size-1) {
            targetSize = size*2;
        } else {
            return;
        }
        if (targetSize < MINIMUM_VECTOR_SIZE) {
            targetSize = MINIMUM_VECTOR_SIZE;
        }
        T* swap = nullptr;
        swap = (T*)Allocator::kalloc(targetSize*sizeof(T));
        if (swap == nullptr) {
            fault(3,"nullptr from kalloc in vector.");
        }
        kmemset(swap,0,targetSize*sizeof(T));
        kmemcpy(swap,array,length*sizeof(T));
        if (array != nullptr) {
            Allocator::free(array);
            array = nullptr;
        }
        array = swap;
        swap = nullptr;
        size = targetSize;
    }
    void steal(Vector&& other) {
        array = other.array;
        length = other.length;
        size = other.size;
        other.array = nullptr;
        other.length = 0;
        other.size = 0;
    }
    void add(T i) {
        if (array == nullptr) return;
        resize();
        array[length] = i;
        length++;
    }
    void remove(size_t index) {
        if (array == nullptr) return;
        if (index >= length) return;
        kmemmove(array+index,array+index+1,(length-(index+1))*sizeof(T));
        length--;
        array[length] = T{};
        resize();
    }
    void open() {
        length = 0;
        size = MINIMUM_VECTOR_SIZE;
        array = (T*)Allocator::kalloc(size*sizeof(T));
        if (array == nullptr) return;
        kmemset(array,0,size*sizeof(T));
    }
    Vector() = default;
    Vector(Vector&& other) noexcept {
        steal((Vector&&)other);
    }
    Vector& operator=(Vector&& other) {
        if (this == &other) return *this;
        if (array != nullptr) {
            Allocator::free(array);
        }
        steal((Vector&&)other);
        return *this;
    }
    T& operator[](size_t index) {
        return array[index];
    }
    void close() {
        if (array == nullptr) return;
        Allocator::free(array);
        array = nullptr;
    }
    T* begin() {
        return array;
    }

    T* end() {
        return array + length;
    }

    const T* begin() const {
        return array;
    }

    const T* end() const {
        return array + length;
    }
};