#ifndef STDIO_H
#define STDIO_H

#include <stdarg.h>
#include <stdint.h>

namespace uqaabOS
{
  namespace libc
  {
    void putchar(char);
    void puts(const char *);
    void print_int(int);
    void print_hex(unsigned long);
    void printf(const char *, ...);
    void init_cursor();
    void move_cursor(int dx, int dy);
  } // namespace libc

} // namespace uqaabOS
#endif // PRINTF_H
