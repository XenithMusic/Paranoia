#include <stddef.h>

extern "C" {
    size_t strlen(const char* str);
    bool stringSafe(const char* str);
    char* stringConcat(const char* a, const char* b);
    char* parseInt(int num, char* str, int base);
    char* parseDouble(double num, char* str, int precision);
}