// GCC provides these header files automatically
#include "include/gdt.h"
#include "include/libc/stdio.h"

// Compiler checks
#if defined(__linux__)
#error "This code must be compiled with a cross-compiler"
#elif !defined(__i386__)
#error "This code must be compiled with an x86-elf compiler"
#endif

// Kernel entry point
extern "C" void kernel_main() {
  // Create our terminal instance
  uqaabOS::libc::Terminal terminal;

  // Display some messages
  terminal.print("Hello, World!\n");
  terminal.print("Welcome to the C++ kernel.\n");

  // Initialize Global Descriptor Table in kernel
  uqaabOS::include::GDT gdt;

  terminal.print("Loaded GDT....\n");
  terminal.print("Running......");

  while (1)
    ;
}
