#include "../include/libc/stdio.h"

namespace uqaabOS {
namespace libc {
// VGA text buffer address
volatile char *video = (volatile char *)0xB8000;
static int cursor_pos = 0;
const int SCREEN_WIDTH = 80;
const int SCREEN_HEIGHT = 25;
const int SCREEN_SIZE = SCREEN_WIDTH * SCREEN_HEIGHT;

// Write a byte to a port
void outb(uint16_t port, uint8_t data) {
    __asm__ volatile ("outb %0, %1" : : "a"(data), "Nd"(port));
}

void update_hw_cursor(int x, int y) {
  // Calculate the cursor's position in the VGA text buffer
  uint16_t cursorLocation = y * SCREEN_WIDTH + x;

  // Send the high byte of the cursor location to the VGA control register
  outb(0x3D4, 0x0E); 
  outb(0x3D5, (cursorLocation >> 8) & 0xFF);

  // Send the low byte of the cursor location to the VGA control register
  outb(0x3D4, 0x0F); 
  outb(0x3D5, cursorLocation & 0xFF);
}

// Scrolls the screen content up by one line
void scroll_screen() {
    // Copy content of lines 1 to SCREEN_HEIGHT-1 up to lines 0 to SCREEN_HEIGHT-2
    for (int pos = 0; pos < (SCREEN_HEIGHT - 1) * SCREEN_WIDTH; ++pos) {
        video[pos * 2] = video[(pos + SCREEN_WIDTH) * 2];
        video[pos * 2 + 1] = video[(pos + SCREEN_WIDTH) * 2 + 1];
    }

    // Clear the last line
    int last_line_start_index = (SCREEN_HEIGHT - 1) * SCREEN_WIDTH;
    for (int x = 0; x < SCREEN_WIDTH; ++x) {
        video[(last_line_start_index + x) * 2] = ' '; // Clear character
        video[(last_line_start_index + x) * 2 + 1] = 0x07; // Default attribute (white on black)
    }
}

void putchar(char c) {
    // Handle newline character
    if (c == '\n') {
        cursor_pos += SCREEN_WIDTH - (cursor_pos % SCREEN_WIDTH);
    }
    // Handle backspace character
    else if (c == '\b') {
        if (cursor_pos > 0) {
            cursor_pos--; // Move cursor back
            video[cursor_pos * 2] = ' '; // Clear the character at the cursor position
            video[cursor_pos * 2 + 1] = 0x07; // Reset to default attribute (white on black)
        }
    }
    // Handle regular printable characters
    else if (c >= ' ') {
        // If cursor is already at or beyond the screen limit before printing, scroll first.
        if (cursor_pos >= SCREEN_SIZE) {
            scroll_screen();
            cursor_pos -= SCREEN_WIDTH;
        }

        video[cursor_pos * 2] = c;
        video[cursor_pos * 2 + 1] = 0x07; // Default attribute (white on black)
        cursor_pos++;
    }

    // Update the hardware cursor position
    update_hw_cursor(cursor_pos % SCREEN_WIDTH, cursor_pos / SCREEN_WIDTH);
}

void puts(const char *str) {
  while (*str) {
    putchar(*str++);
  }
}

void print_int(int num) {
  char buffer[16];
  int i = 0;
  bool is_negative = false;

  if (num == 0) {
    putchar('0');
    return;
  }

  if (num < 0) {
    is_negative = true;
    // Handle potential overflow for INT_MIN
    if (num == -2147483648) {
         puts("-2147483648");
         return;
    }
    num = -num;
  }

  while (num > 0) {
    buffer[i++] = '0' + (num % 10);
    num /= 10;
  }

  if (is_negative) {
      putchar('-');
  }

  while (i > 0) {
    putchar(buffer[--i]);
  }
}

void print_hex(uint32_t num) {
  char buffer[16];
  int i = 0;
  puts("0x");

  if (num == 0) {
      putchar('0');
      return;
  }

  do {
    int digit = num % 16;
    buffer[i++] = (digit < 10) ? ('0' + digit) : ('A' + (digit - 10));
    num /= 16;
  } while (num > 0);

  // Print leading zeros if needed for fixed width, e.g., 8 hex digits
  // while(i < 8) buffer[i++] = '0';

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
      case 's': {
        char *s = va_arg(args, char *);
        if (s == nullptr) { // Handle null pointers gracefully
            puts("(null)");
        } else {
            puts(s);
        }
        break;
      }
      case 'd':
        print_int(va_arg(args, int));
        break;
      case 'x':
        print_hex(va_arg(args, uint32_t));
        break;
      case 'c':
        // char is promoted to int when passed through ...
        putchar((char)va_arg(args, int));
        break;
      case '%':
        putchar('%');
        break;
      default:
        // Print unknown format specifier literally
        putchar('%');
        putchar(*format);
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
