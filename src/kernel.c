#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// First, let's do some basic checks to make sure we are using our x86-elf
// cross-compiler correctly
#if defined(__linux__)
#error "This code must be compiled with a cross-compiler"
#elif !defined(__i386__)
#error "This code must be compiled with an x86-elf compiler"
#endif

/* VGA constants */
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY (uint16_t *)0xB8000

/* PS/2 keyboard I/O ports */
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

static inline uint8_t inb(uint16_t port) {
  uint8_t result;
  asm volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
  return result;
}

/* VGA color definitions */
enum vga_color {
  VGA_COLOR_BLACK = 0,
  VGA_COLOR_BLUE = 1,
  VGA_COLOR_GREEN = 2,
  VGA_COLOR_CYAN = 3,
  VGA_COLOR_RED = 4,
  VGA_COLOR_MAGENTA = 5,
  VGA_COLOR_BROWN = 6,
  VGA_COLOR_LIGHT_GREY = 7,
  VGA_COLOR_DARK_GREY = 8,
  VGA_COLOR_LIGHT_BLUE = 9,
  VGA_COLOR_LIGHT_GREEN = 10,
  VGA_COLOR_LIGHT_CYAN = 11,
  VGA_COLOR_LIGHT_RED = 12,
  VGA_COLOR_LIGHT_MAGENTA = 13,
  VGA_COLOR_LIGHT_BROWN = 14,
  VGA_COLOR_WHITE = 15,
};

/* Global variables */
static size_t terminal_row = 0;
static size_t terminal_column = 0;
static uint8_t terminal_color;
static uint16_t *terminal_buffer = VGA_MEMORY;

/* Map Scan Code Set 2 for printable keys */
static char scancode_to_char[256] = {
    [0x1E] = 'a', [0x30] = 'b', [0x2E] = 'c', [0x20] = 'd', [0x12] = 'e',
    [0x21] = 'f', [0x22] = 'g', [0x23] = 'h', [0x17] = 'i', [0x24] = 'j',
    [0x25] = 'k', [0x26] = 'l', [0x32] = 'm', [0x31] = 'n', [0x18] = 'o',
    [0x19] = 'p', [0x10] = 'q', [0x13] = 'r', [0x1F] = 's', [0x14] = 't',
    [0x16] = 'u', [0x2F] = 'v', [0x11] = 'w', [0x2D] = 'x', [0x15] = 'y',
    [0x2C] = 'z', [0x02] = '1', [0x03] = '2', [0x04] = '3', [0x05] = '4',
    [0x06] = '5', [0x07] = '6', [0x08] = '7', [0x09] = '8', [0x0A] = '9',
    [0x0B] = '0', [0x39] = ' ', [0x1C] = '\n' // Space and Enter
};

/* VGA functions */
void terminal_initialize(void) {
  terminal_color = VGA_COLOR_LIGHT_GREY | (VGA_COLOR_BLACK << 4);
  terminal_buffer = VGA_MEMORY;
  for (size_t y = 0; y < VGA_HEIGHT; y++) {
    for (size_t x = 0; x < VGA_WIDTH; x++) {
      terminal_buffer[y * VGA_WIDTH + x] =
          (uint16_t)' ' | (uint16_t)terminal_color << 8;
    }
  }
}

void terminal_putchar(char c) {
  if (c == '\n') {
    terminal_column = 0;
    if (++terminal_row == VGA_HEIGHT)
      terminal_row = 0;
  } else {
    terminal_buffer[terminal_row * VGA_WIDTH + terminal_column] =
        (uint16_t)c | (uint16_t)terminal_color << 8;
    if (++terminal_column == VGA_WIDTH) {
      terminal_column = 0;
      if (++terminal_row == VGA_HEIGHT)
        terminal_row = 0;
    }
  }
}

void terminal_writestring(const char *str) {
  for (size_t i = 0; str[i] != '\0'; i++) {
    terminal_putchar(str[i]);
  }
}

/* Keyboard Input Functions */
uint8_t keyboard_read_scancode() {
  while (true) {
    if (inb(KEYBOARD_STATUS_PORT) & 0x01) { // Check if data is available
      return inb(KEYBOARD_DATA_PORT);
    }
  }
}

char keyboard_getchar() {
  uint8_t scancode = keyboard_read_scancode();
  return scancode_to_char[scancode & 0x7F]; // Mask out break code bit
}

/* Kernel entry point */
void kernel_main(void) {
  terminal_initialize();
  terminal_writestring("Basic Kernel: Type something!\n");

  while (true) {
    char c = keyboard_getchar();
    if (c) {
      terminal_putchar(c);
    }
  }
}
