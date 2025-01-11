#ifndef STDIO_H
#define STDIO_H

#include <stddef.h>
#include <stdint.h>

namespace uqaabOS {
namespace libc {
class Terminal {
public:
  static const int VGA_COLS = 80;
  static const int VGA_ROWS = 25;

private:
  volatile uint16_t *vga_buffer;
  int term_col;
  int term_row;
  uint8_t term_color;

public:
  Terminal();
  ~Terminal();

  void putChar(char c);      // Print a single character
  void print(const char *s); // Print a string
  void clear();              // Clear the screen
};
} // namespace libc
} // namespace uqaabOS

#endif
