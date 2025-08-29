// GCC provides these header files automatically
#include "include/drivers/driver.h"
#include "include/drivers/keyboard.h"
#include "include/drivers/mouse.h"
#include "include/drivers/pci.h"
// #include "include/drivers/vga.h"
#include "include/drivers/storage/ata.h"
#include "include/filesystem/fat32.h"
#include "include/filesystem/msdospart.h"
#include "include/gdt.h"
#include "include/interrupts.h"
#include "include/libc/stdio.h"
#include "include/memorymanagement/memorymanagement.h"
#include "include/multitasking/multitasking.h"
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
  for (int i = 0; i < 100; i++) {
    uqaabOS::libc::printf("A");
    // Yield every 10 iterations
    if (i % 10 == 9) {
      asm volatile("int $0x20");
    }
  }
}

void taskB() {
  for (int i = 0; i < 100; i++) {
    uqaabOS::libc::printf("B");
    // Yield every 10 iterations
    if (i % 10 == 9) {
      asm volatile("int $0x20");
    }
  }
}

// Kernel entry point
extern "C" void kernel_main(const void *multiboot_structure,
                            uint32_t /*multiboot_magic*/) {
  uqaabOS::libc::printf("Hello, World!\n");

  // Initialize Global Descriptor Table in kernel
  uqaabOS::libc::printf("Initializing GDT...\n");
  uqaabOS::include::GDT gdt;
  uqaabOS::libc::printf("Loaded GDT....\n");

  // Initialize MemoryManager
  uqaabOS::libc::printf("Initializing MemoryManager...\n");
  uint32_t *memupper =
      (uint32_t *)(((uqaabOS::memorymanagement::size_t)multiboot_structure) +
                   8);
  size_t heap = 10 * 1024 * 1024;
  uqaabOS::memorymanagement::MemoryManager memoryManager(
      heap, (*memupper) * 1024 - heap - 10 * 1024);
  uqaabOS::libc::printf("MemoryManager initialized.\n");

  uqaabOS::libc::printf("heap: ");
  uqaabOS::libc::print_hex(heap);

  void *allocated = memoryManager.malloc(1024);
  uqaabOS::libc::printf("\nallocated: ");
  uqaabOS::libc::print_hex((size_t)allocated);
  uqaabOS::libc::printf("\n");

  // Initialize TaskManager
  uqaabOS::libc::printf("Initializing TaskManager...\n");
  uqaabOS::multitasking::TaskManager task_manager;
  uqaabOS::libc::printf("TaskManager initialized.\n");

  // For now, we're not adding any tasks to avoid freezing the system
  // In a real implementation, we would need to properly manage task memory

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

  // Activate drivers and interrupts before file operations
  // This ensures hardware operations can use interrupts if needed
  uqaabOS::libc::printf("Activating drivers...\n");
  driver_manager.activate_all();
  uqaabOS::libc::printf("Activating interrupts...\n");
  interrupt_manager.activate();
  uqaabOS::libc::printf("Interrupts activated.\n");

  uqaabOS::libc::printf("\n ATA primary master: ");
  uqaabOS::driver::ATA ata0m(true, 0x1F0);
  ata0m.identify();

  // uqaabOS::libc::printf("\n ATA primary slave: ");
  // uqaabOS::driver::ATA ata0s(false, 0x1F0);
  // ata0s.identify();

  // Read partitions
  uqaabOS::filesystem::MSDOSPartitionTable::read_partitions(&ata0m);
  uqaabOS::libc::printf("\n");

  // Test our new FAT32 implementation
  uint32_t fat32_lba =
      uqaabOS::filesystem::MSDOSPartitionTable::get_first_fat32_partition_lba(
          &ata0m);
  if (fat32_lba == 0) {
    uqaabOS::libc::printf("No FAT32 partition found or invalid MBR\n");
  } else {
    uqaabOS::libc::printf("Found FAT32 partition at LBA: ");
    uqaabOS::libc::print_hex(fat32_lba);
    uqaabOS::libc::printf("\n");

    uqaabOS::filesystem::FAT32 fat32(&ata0m, fat32_lba);
    if (fat32.initialize()) {
      uqaabOS::libc::printf("FAT32 filesystem initialized successfully\n");
      uqaabOS::libc::printf("Root directory contents:\n");
      fat32.list_root();

      // Test our new functions
      uqaabOS::libc::printf("\n Testing new FAT32 functions: \n");

      // Test ls function
      uqaabOS::libc::printf("\n Listing directory with ls(): \n");
      uqaabOS::libc::printf("\n /test: ");
      fat32.ls("/test");
      uqaabOS::libc::printf("\n");
      uqaabOS::libc::printf("\n Root: ");
      fat32.ls("/");
      uqaabOS::libc::printf("\n");

      // Test case-insensitive file opening with different case variations
      // Try to open a file with lowercase name - this should work regardless of
      // actual case on disk
      const char *filename = "hello.txt"; // Use lowercase as requested

      uqaabOS::libc::printf("\n Trying to open file with name: ");
      uqaabOS::libc::printf(filename);
      uqaabOS::libc::printf("\n");

      int fd = fat32.open(filename);
      if (fd >= 0) {
        uqaabOS::libc::printf("Successfully opened file: ");
        uqaabOS::libc::printf(filename);
        uqaabOS::libc::printf("\n");

        // Read some content from the file
        uint8_t buffer[512];
        int bytes = fat32.read(fd, buffer, 512);
        if (bytes > 0) {
          // Ensure null termination for printing
          if (bytes < 512)
            buffer[bytes] = '\0';
          else
            buffer[511] = '\0';

          uqaabOS::libc::printf("Read %d bytes from file: \n", bytes);
          uqaabOS::libc::printf((char *)buffer);
          uqaabOS::libc::printf("\n");
        }
        fat32.close(fd);
      } else {
        uqaabOS::libc::printf("Failed to open file: ");
        uqaabOS::libc::printf(filename);
        uqaabOS::libc::printf("\n");
      }
    } else {
      uqaabOS::libc::printf("Failed to initialize FAT32 filesystem\n");
    }
  }
  uqaabOS::libc::printf("\n \n");

  // vga graphics mode
  // vga.set_mode(320, 200, 8);
  // for (int32_t y = 0; y < 200; y++)
  //   for (int32_t x = 0; x < 320; x++)
  //     vga.put_pixel(x, y, 0x00, 0xFF, 0x00);

  while (1)
    ;
}