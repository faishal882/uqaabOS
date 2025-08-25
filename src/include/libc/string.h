#ifndef STRING_H
#define STRING_H

#include <stdint.h>

namespace uqaabOS {
namespace libc {

void* memset(void* dest, int value, uint32_t count);
void* memcpy(void* dest, const void* src, uint32_t count);
int strcmp(const char* str1, const char* str2);
int strncmp(const char* str1, const char* str2, uint32_t n);
char* strncpy(char* dest, const char* src, uint32_t n);
uint32_t strlen(const char* str);

} // namespace libc
} // namespace uqaabOS

#endif // STRING_H