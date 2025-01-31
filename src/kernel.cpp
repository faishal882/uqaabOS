// GCC provides these header files automatically
#include "include/drivers/driver.h"
#include "include/drivers/keyboard.h"
#include "include/drivers/mouse.h"
#include "include/drivers/pci.h"
#include "include/drivers/vga.h"

#include "include/gdt.h"
#include "include/interrupts.h"
#include "include/libc/stdio.h"
#include "include/multitasking/multitasking.h"

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

void taskA() {
  for (int i = 0; i < 500; i++) {
    uqaabOS::libc::printf("A");
  }

  return;
}

void taskB() {
  for (int j = 0; j < 500; j++) {
    uqaabOS::libc::printf("B");
  }

  return;
}

void taskC() {
  for (int k = 0; k < 500; k++) {
    uqaabOS::libc::printf("C");
  }

  return;
}

// Kernel entry point
extern "C" void kernel_main() {
  uqaabOS::libc::printf("Hello, World!\n");

  // Initialize Global Descriptor Table in kernel
  uqaabOS::include::GDT gdt;
  uqaabOS::libc::printf("Loaded GDT....\n");

  // Initialize TaskManager
  uqaabOS::multitasking::TaskManager task_manager;

  // Initialize Task
  uqaabOS::multitasking::Task task1(&gdt, taskA);
  uqaabOS::multitasking::Task task2(&gdt, taskB);
  uqaabOS::multitasking::Task task3(&gdt, taskC);
  // uqaabOS::multitasking::Task task4(&gdt, taskD);

  // Add tasks to task manager
  task_manager.add_task(&task1);
  task_manager.add_task(&task2);
  task_manager.add_task(&task3);
  // task_manager.add_task(&task4);

  // Initialize InterruptManager with TaskManager
  uqaabOS::interrupts::InterruptManager interrupt_manager(0x20, &gdt,
                                                          &task_manager);
  interrupt_manager.activate();

  // Start multitasking
  while (true)
    ;
}
