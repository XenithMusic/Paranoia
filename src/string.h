#include <stddef.h>

extern "C" {
    size_t strlen(const char* str);
    bool stringSafe(const char* str);
    char* stringConcat(const char* a, const char* b);
    char* parseInt(int inNum, char* str, int base);
    char* parseU32(uint32_t inNum, char* str, int base);
    char* parseDouble(double inNum, char* str, int precision);
}