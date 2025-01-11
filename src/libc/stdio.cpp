#include "../include/libc/stdio.h"

namespace uqaabOS {
namespace libc {

// Constructor
Terminal::Terminal()
    : vga_buffer(reinterpret_cast<uint16_t *>(0xB8000)), term_col(0),
      term_row(0), term_color(0x0F) // Black background, White foreground
{
  clear();
}

// Clear the screen
void Terminal::clear() {
  for (int row = 0; row < VGA_ROWS; ++row) {
    for (int col = 0; col < VGA_COLS; ++col) {
      const size_t index = (VGA_COLS * row) + col;
      vga_buffer[index] = static_cast<uint16_t>(term_color << 8) | ' ';
    }
  }
  term_col = 0;
  term_row = 0;
}

// Print a single character
void Terminal::putChar(char c) {
  switch (c) {
  case '\n':
    term_col = 0;
    ++term_row;
    break;
  default:
    const size_t index = (VGA_COLS * term_row) + term_col;
    vga_buffer[index] = static_cast<uint16_t>(term_color << 8) | c;
    ++term_col;
    break;
  }

  // Handle column overflow
  if (term_col >= VGA_COLS) {
    term_col = 0;
    ++term_row;
  }

  // Handle row overflow by scrolling
  if (term_row >= VGA_ROWS) {
    term_row = VGA_ROWS - 1;
    for (int row = 0; row < VGA_ROWS - 1; ++row) {
      for (int col = 0; col < VGA_COLS; ++col) {
        const size_t index = (VGA_COLS * row) + col;
        const size_t next_index = (VGA_COLS * (row + 1)) + col;
        vga_buffer[index] = vga_buffer[next_index];
      }
    }
    // Clear the last row
    for (int col = 0; col < VGA_COLS; ++col) {
      const size_t index = (VGA_COLS * (VGA_ROWS - 1)) + col;
      vga_buffer[index] = static_cast<uint16_t>(term_color << 8) | ' ';
    }
  }
}

// Print a string
void Terminal::print(const char *s) {
  for (size_t i = 0; s[i] != '\0'; ++i) {
    putChar(s[i]);
  }
}

// Destructor
Terminal::~Terminal() {
  // do nothing
}

} // namespace libc
} // namespace uqaabOS
