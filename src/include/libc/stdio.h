#ifndef STDIO_H
#define STDIO_H

#include <stdarg.h>
#include <stdint.h>

namespace uqaabOS {
namespace libc {
void printf(const char *format, ...);
void putchar(char c);
void puts(const char *str);
void print_int(int num);
void print_hex(uint32_t num);
} // namespace libc
} // namespace uqaabOS
#endif // PRINTF_H
