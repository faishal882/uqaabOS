#include "include/drivers/driver.h"
#include "include/drivers/keyboard.h"
#include "include/drivers/mouse.h"
#include "include/drivers/pci.h"
#include "include/drivers/vga.h"
#include "include/gdt.h"
#include "include/gui/desktop.h"
#include "include/gui/window.h"
#include "include/interrupts.h"
#include "include/libc/stdio.h"

#define GRAPHICSMODE

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
  // Initialize Global Descriptor Table in kernel
  uqaabOS::include::GDT gdt;
  uqaabOS::libc::printf("Loaded GDT....\n");

  uqaabOS::interrupts::InterruptManager interrupts(0x20, &gdt);
  uqaabOS::libc::printf("Loaded Interrupts....\n");

  uqaabOS::libc::printf("Initializing Hardware, Stage 1\n");

#ifdef GRAPHICSMODE
  uqaabOS::gui::Desktop desktop(320, 200, 0x00, 0x00, 0xA8);
#endif

  uqaabOS::driver::DriverManager driver_manager;

#ifdef GRAPHICSMODE
  uqaabOS::driver::MouseDriver mouse(&interrupts, &desktop);
#else
  MouseToConsole mouse_event_driver;
  uqaabOS::driver::MouseDriver mouse(&interrupts, &mouse_event_driver);
#endif
  driver_manager.add_driver(&mouse);

#ifdef GRAPHICSMODE
  uqaabOS::driver::KeyboardDriver keyboard(&interrupts, &desktop);
#else
  PrintfKeyboardEventHandler keyboard_event_driver;
  uqaabOS::driver::KeyboardDriver keyboard(&interrupts, &keyboard_event_driver);
#endif
  driver_manager.add_driver(&keyboard);

  uqaabOS::driver::PCIController pci_controller;
  pci_controller.select_drivers(&driver_manager, &interrupts);

  uqaabOS::driver::VideoGraphicsArray vga;

  uqaabOS::libc::printf("Initializing Hardware, Stage 2\n");
  driver_manager.activate_all();

  uqaabOS::libc::printf("Initializing Hardware, Stage 3\n");

#ifdef GRAPHICSMODE
  vga.set_mode(320, 200, 8);
  uqaabOS::gui::Window win1(&desktop, 10, 10, 20, 20, 0xFF, 0x00, 0x00);
  desktop.add_child(&win1);
  uqaabOS::gui::Window win2(&desktop, 40, 15, 30, 30, 0x00, 0xFF, 0x00);
  desktop.add_child(&win2);
#endif

  interrupts.activate();

  // should raise a divide by zero exception
  // uqaabOS::libc::print_int(10 / 1);

  vga.set_mode(320, 200, 8);
  for (int32_t y = 0; y < 200; y++)
    for (int32_t x = 0; x < 320; x++)
      vga.put_pixel(x, y, 0xFF, 0xFF, 0xFF);

  while (1) {
#ifdef GRAPHICSMODE
    desktop.Draw(&vga);
#endif
  }
}
