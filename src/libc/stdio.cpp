#include "../include/libc/stdio.h"

namespace uqaabOS {
namespace libc {
// VGA text buffer address
volatile char *video = (volatile char *)0xB8000;
static int cursor_pos = 0;

void putchar(char c) {
  if (c == '\n') {
    cursor_pos += 80 - (cursor_pos % 80);
  } else {
    video[cursor_pos * 2] = c;
    video[cursor_pos * 2 + 1] = 0x07;
    cursor_pos++;
  }
}

void puts(const char *str) {
  while (*str) {
    putchar(*str++);
  }
}

void print_int(int num) {
  char buffer[16];
  int i = 0;
  if (num < 0) {
    putchar('-');
    num = -num;
  }
  do {
    buffer[i++] = '0' + (num % 10);
    num /= 10;
  } while (num > 0);

  while (i > 0) {
    putchar(buffer[--i]);
  }
}

void print_hex(uint32_t num) {
  char buffer[16];
  int i = 0;
  puts("0x");
  do {
    int digit = num % 16;
    buffer[i++] = (digit < 10) ? ('0' + digit) : ('A' + (digit - 10));
    num /= 16;
  } while (num > 0);

  while (i > 0) {
    putchar(buffer[--i]);
  }
}

void printf(const char *format, ...) {
  va_list args;
  va_start(args, format);

  while (*format) {
    if (*format == '%') {
      format++;
      switch (*format) {
      case 's':
        puts(va_arg(args, char *));
        break;
      case 'd':
        print_int(va_arg(args, int));
        break;
      case 'x':
        print_hex(va_arg(args, uint32_t));
        break;
      case 'c':
        putchar((char)va_arg(args, int));
        break;
      case '%':
        putchar('%');
        break;
      default:
        putchar('?');
        break;
      }
    } else {
      putchar(*format);
    }
    format++;
  }

  va_end(args);
}
} // namespace libc
} // namespace uqaabOS
