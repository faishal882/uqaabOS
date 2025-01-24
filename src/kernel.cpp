// GCC provides these header files automatically
#include "include/drivers/driver.h"
#include "include/drivers/keyboard.h"
#include "include/drivers/mouse.h"
#include "include/drivers/pci.h"
#include "include/drivers/vga.h"
#include "include/gdt.h"
#include "include/interrupts.h"
#include "include/libc/stdio.h"

// #include "include/keyboard/keyboard.h"

// Compiler checks
#if defined(__linux__)
#error "This code must be compiled with a cross-compiler"
#elif !defined(__i386__)
#error "This code must be compiled with an x86-elf compiler"
#endif

class PrintfKeyboardEventHandler
    : public uqaabOS::driver::KeyboardEventHandler {
public:
  void on_key_down(char c) {
    char foo[] = " ";
    foo[0] = c;
    uqaabOS::libc::printf(foo);
  }
};

class MouseToConsole : public uqaabOS::driver::MouseEventHandler {
  int8_t x, y;

public:
  MouseToConsole() {}

  virtual void on_activate() {
    uint16_t *VideoMemory = (uint16_t *)0xb8000;
    x = 40;
    y = 12;
    VideoMemory[80 * y + x] = (VideoMemory[80 * y + x] & 0x0F00) << 4 |
                              (VideoMemory[80 * y + x] & 0xF000) >> 4 |
                              (VideoMemory[80 * y + x] & 0x00FF);
  }

  virtual void on_mouse_move(int xoffset, int yoffset) {
    static uint16_t *VideoMemory = (uint16_t *)0xb8000;
    VideoMemory[80 * y + x] = (VideoMemory[80 * y + x] & 0x0F00) << 4 |
                              (VideoMemory[80 * y + x] & 0xF000) >> 4 |
                              (VideoMemory[80 * y + x] & 0x00FF);

    x += xoffset;
    if (x >= 80)
      x = 79;
    if (x < 0)
      x = 0;
    y += yoffset;
    if (y >= 25)
      y = 24;
    if (y < 0)
      y = 0;

    VideoMemory[80 * y + x] = (VideoMemory[80 * y + x] & 0x0F00) << 4 |
                              (VideoMemory[80 * y + x] & 0xF000) >> 4 |
                              (VideoMemory[80 * y + x] & 0x00FF);
  }
};

// Kernel entry point
extern "C" void kernel_main() {
  uqaabOS::libc::printf("Hello, World!\n");

  // Initialize Global Descriptor Table in kernel
  uqaabOS::include::GDT gdt;
  uqaabOS::libc::printf("Loaded GDT....\n");

  // Initialize Interrupts
  uqaabOS::interrupts::InterruptManager interrupts(0x20, &gdt);

  uqaabOS::driver::DriverManager driver_manager;

  MouseToConsole mouse_event_driver;
  uqaabOS::driver::MouseDriver mouse(&interrupts, &mouse_event_driver);
  driver_manager.add_driver(&mouse);

  PrintfKeyboardEventHandler keyboard_event_driver;
  uqaabOS::driver::KeyboardDriver keyboard(&interrupts, &keyboard_event_driver);
  driver_manager.add_driver(&keyboard);

  uqaabOS::driver::PCIController pci_controller;
  pci_controller.select_drivers(&driver_manager , &interrupts);

  uqaabOS::driver::VideoGraphicsArray vga;

  driver_manager.activate_all();

  // driver_manager.add_driver(&keyboard);
  // driver_manager.add_driver(&mouse);

  interrupts.activate();

  // should raise a divide by zero exception
  uqaabOS::libc::print_int(10 / 1);

  vga.set_mode(320, 200, 8);
  for (int32_t y = 0; y < 200; y++)
    for (int32_t x = 0; x < 320; x++)
      vga.put_pixel(x, y, 0x00, 0xFF, 0x00);
  while (1)
    ;
}
