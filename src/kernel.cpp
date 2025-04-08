// GCC provides these header files automatically
#include "include/drivers/driver.h"
#include "include/drivers/keyboard.h"
#include "include/drivers/mouse.h"
#include "include/drivers/pci.h"
// #include "include/drivers/vga.h"
#include "include/drivers/storage/ata.h"
#include "include/gdt.h"
#include "include/interrupts.h"
#include "include/libc/stdio.h"
#include "include/memorymanagement/memorymanagement.h"
#include "include/multitasking/multitasking.h"
#include "include/filesystem/msdospart.h"
#include "include/filesystem/fat.h"
#include <cstdint>

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
  int i = 0;
  while (1) {
    i++;
    uqaabOS::libc::printf("A");
    if (i % 10 == 0) {
      asm volatile("int $0x20");
    }
  }

  asm volatile("int $0x20");
}

void taskB() {
  for (int i = 0; i < 100000000; i++) {
    uqaabOS::libc::printf("B");
    if (i % 10 == 0) {
      asm volatile("int $0x20");
    }
  }
  asm volatile("int $0x20");
}

void taskC() {
  for (int i = 0; i < 100000000; i++) {
    uqaabOS::libc::printf("C");
    if (i % 10 == 0) {
      asm volatile("int $0x20");
    }
  }
  asm volatile("int $0x20");
}

void taskD() {
  for (int i = 0; i < 100000000; i++) {
    uqaabOS::libc::printf("D");
    if (i % 10 == 0) {
      asm volatile("int $0x20");
    }
  }
  asm volatile("int $0x20");
}

// Kernel entry point
extern "C" void kernel_main(const void *multiboot_structure,
                            uint32_t /*multiboot_magic*/) {
  uqaabOS::libc::printf("Hello, World!\n");

  // Initialize Global Descriptor Table in kernel
  uqaabOS::include::GDT gdt;
  uqaabOS::libc::printf("Loaded GDT....\n");

  // Initialize MemoryManager
  uint32_t *memupper =
      (uint32_t *)(((uqaabOS::memorymanagement::size_t)multiboot_structure) +
                   8);
  size_t heap = 10 * 1024 * 1024;
  uqaabOS::memorymanagement::MemoryManager memoryManager(
      heap, (*memupper) * 1024 - heap - 10 * 1024);

  uqaabOS::libc::printf("heap: ");
  uqaabOS::libc::print_hex(heap);

  void *allocated = memoryManager.malloc(1024);
  uqaabOS::libc::printf("\nallocated: ");
  uqaabOS::libc::print_hex((size_t)allocated);
  uqaabOS::libc::printf("\n");

  // Initialize TaskManager
  uqaabOS::multitasking::TaskManager task_manager;

  // Initialize Task
  // uqaabOS::multitasking::Task task1(&gdt, taskA);
  // uqaabOS::multitasking::Task task2(&gdt, taskB);
  // uqaabOS::multitasking::Task task3(&gdt, taskC);
  // uqaabOS::multitasking::Task task4(&gdt, taskD);

  // // Add tasks to task manager
  // task_manager.add_task(&task1);
  // task_manager.add_task(&task2);
  // task_manager.add_task(&task3);
  // task_manager.add_task(&task4);

  uqaabOS::libc::printf("\n ATA primary master: ");
  uqaabOS::driver::ATA ata0m(true, 0x1F0);
  ata0m.identify();
  
  // uqaabOS::libc::printf("\n ATA primary slave: ");
  // uqaabOS::driver::ATA ata0s(false, 0x1F0);
  // ata0s.identify();

 // Read partitions
 uqaabOS::filesystem::MSDOSPartitionTable::read_partitions(&ata0m);
 uqaabOS::libc::printf("\n");

 // Calculate partition offset in bytes (sector size is typically 512 bytes)
 uint32_t partition_offset = 2048;
 // Read the FAT32 filesystem
 uqaabOS::filesystem::read_bios_parameter_block(&ata0m, partition_offset);
 uqaabOS::libc::printf("\n \n");

  // Initialize InterruptManager with TaskManager
  uqaabOS::interrupts::InterruptManager interrupt_manager(0x20, &gdt,
                                                          &task_manager);

  uqaabOS::driver::DriverManager driver_manager;

  MouseToConsole mouse_event_driver;
  uqaabOS::driver::MouseDriver mouse(&interrupt_manager, &mouse_event_driver);
  driver_manager.add_driver(&mouse);

  PrintfKeyboardEventHandler keyboard_event_driver;
  uqaabOS::driver::KeyboardDriver keyboard(&interrupt_manager,
                                           &keyboard_event_driver);
  driver_manager.add_driver(&keyboard);

  uqaabOS::driver::PCIController pci_controller;
  pci_controller.select_drivers(&driver_manager, &interrupt_manager);

  // uqaabOS::driver::VideoGraphicsArray vga;

  driver_manager.activate_all();

  interrupt_manager.activate();

  // should raise a divide by zero exception
  uqaabOS::libc::print_int(10 / 1);

  // vga graphics mode
  // vga.set_mode(320, 200, 8);
  // for (int32_t y = 0; y < 200; y++)
  //   for (int32_t x = 0; x < 320; x++)
  //     vga.put_pixel(x, y, 0x00, 0xFF, 0x00);

  while (1)
    ;
}