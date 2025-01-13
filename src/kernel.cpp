// GCC provides these header files automatically
#include "include/drivers/keyboard.h"
#include "include/drivers/mouse.h"
#include "include/gdt.h"
#include "include/interrupts.h"
#include "include/libc/stdio.h"

// Compiler checks
#if defined(__linux__)
#error "This code must be compiled with a cross-compiler"
#elif !defined(__i386__)
#error "This code must be compiled with an x86-elf compiler"
#endif

// Kernel entry point
extern "C" void kernel_main() {
  uqaabOS::libc::printf("Hello, World!\n");

  // Initialize Global Descriptor Table in kernel
  uqaabOS::include::GDT gdt;
  uqaabOS::libc::printf("Loaded GDT....\n");

  // Initialize Interrupts
  uqaabOS::interrupts::InterruptManager interrupts(0x20, &gdt);
  uqaabOS::driver::MouseDriver mouse(&interrupts);
  uqaabOS::driver::KeyboardDriver keyboard(&interrupts);
  interrupts.activate();

  // should raise a divide by zero exception
  uqaabOS::libc::print_int(10 / 0);
  while (1)
    ;
}
