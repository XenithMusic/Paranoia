#include <stddef.h>
#include "math.h"

/*

Copyright (C) 2024  XenithMusic (on github)

The Paranoia kernel is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Paranoia is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Paranoia. If not, see <https://www.gnu.org/licenses/>.

*/


extern "C" {
    size_t strlen(const char* str) {
        size_t len = 0;
        while (str[len] != '\0') {
            len++;
        }
        return len;
    }
    bool stringSafe(const char* str) {
        size_t i = 0;
        while (str[i] != '\0') {
            if (i == '\0') {
                return true;
            }
        }
        return false;
    }
    char* stringConcat(const char* a, const char* b) {
        // // Calculate the lengths of both strings
        // size_t lenA = strlen(a), lenB = strlen(b);

        // // Allocate a buffer large enough for both strings and the null terminator
        // char* buffer = new char[lenA + lenB + 1];

        // // Copy string `a` to buffer
        // for (size_t i = 0; i < lenA; i++) {
        //     buffer[i] = a[i];
        // }

        // // Append string `b` to buffer
        // for (size_t i = 0; i < lenB; i++) {
        //     buffer[lenA + i] = b[i];
        // }

        // // Null-terminate the new string
        // buffer[lenA + lenB] = '\0';

        // return buffer;
        a = a;
        b = b;
        return 0;
    }
    char* parseInt(int inNum, char* str, int base) {
        if (base < 2 or base > 16) {
            return (char*)"Base bad\n";
        }
        int i = 0;
        int num = inNum;

        if (num == 0) {
            str[i++] = '0';
        } else {
            // Only add negative sign if the number is negative and base is 10
            if (num < 0 and base == 10) {
                str[i++] = '-';
                num = -num;
            }
            
            char digits[] = "0123456789abcdef";
            int j = i;
            while (num > 0) {
                str[j++] = digits[num % base];
                num /= base;
            }

            str[j] = '\0';

            // Reverse the string
            int start = i;
            int end = j - 1;
            while (start < end) {
                char temp = str[start];
                str[start] = str[end];
                str[end] = temp;
                start++;
                end--;
            }
        }

        return str;
    }
    char* parseU32(uint32_t inNum, char* str, int base) {
        if (base < 2 or base > 16) {
            return (char*)"Base bad\n";
        }
        int i = 0;
        uint32_t num = inNum;

        if (num == 0) {
            str[i++] = '0';
        } else {
            char digits[] = "0123456789abcdef";
            int j = i;
            while (num > 0) {
                str[j++] = digits[num % base];
                num /= base;
            }

            str[j] = '\0';

            // Reverse the string
            int start = i;
            int end = j - 1;
            while (start < end) {
                char temp = str[start];
                str[start] = str[end];
                str[end] = temp;
                start++;
                end--;
            }
        }

        return str;
    }

    char* parseDouble(double inNum, char* str, int precision) {
        double num = inNum;
        // Handle negative numbers
        if (num < 0) {
            str[0] = '-';
            num = -num;  // Make the number positive for further processing
        }

        // Convert the integer part to a string
        int intPart = (int)floor(num);  // Get the integer part using floor()
        char* ptr = str + (str[0] == '-' ? 1 : 0);  // Start after the sign if negative

        if (intPart == 0) {
            *ptr++ = '0';  // Explicitly add '0' for zero integer part
        } else {
            // Now, parse the integer part without adding the negative sign again
            parseInt(intPart, ptr, 10);  // Call your parseInt function to convert the integer part
            // Move the pointer after the integer part
            while (*ptr != '\0') ptr++;
        }

        // Add the decimal point
        *ptr++ = '.';

        // Convert the fractional part
        double fracPart = num - intPart;
        for (int i = 0; i < precision; i++) {
            fracPart *= 10;
            int digit = (int)fracPart;
            *ptr++ = '0' + digit;  // Convert digit to character
            fracPart -= digit;     // Remove the integer part of the fraction
        }

        // Null-terminate the string
        *ptr = '\0';

        return str;
    }
}