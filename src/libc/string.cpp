#include "../include/libc/string.h"

namespace uqaabOS {
namespace libc {

void* memset(void* dest, int value, uint32_t count) {
    uint8_t* ptr = (uint8_t*)dest;
    while (count--) {
        *ptr++ = value;
    }
    return dest;
}

void* memcpy(void* dest, const void* src, uint32_t count) {
    uint8_t* dest_ptr = (uint8_t*)dest;
    const uint8_t* src_ptr = (const uint8_t*)src;
    while (count--) {
        *dest_ptr++ = *src_ptr++;
    }
    return dest;
}

int strcmp(const char* str1, const char* str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return *(unsigned char*)str1 - *(unsigned char*)str2;
}

int strncmp(const char* str1, const char* str2, uint32_t n) {
    while (n && *str1 && (*str1 == *str2)) {
        str1++;
        str2++;
        n--;
    }
    if (n == 0) {
        return 0;
    }
    return *(unsigned char*)str1 - *(unsigned char*)str2;
}

char* strncpy(char* dest, const char* src, uint32_t n) {
    char* ret = dest;
    while (n && (*dest++ = *src++)) {
        n--;
    }
    while (n--) {
        *dest++ = '\0';
    }
    return ret;
}

uint32_t strlen(const char* str) {
    uint32_t len = 0;
    while (str[len]) {
        len++;
    }
    return len;
}

char* strchr(const char* str, int c) {
    while (*str != (char)c) {
        if (!*str++) {
            return 0;
        }
    }
    return (char*)str;
}

} // namespace libc
} // namespace uqaabOS