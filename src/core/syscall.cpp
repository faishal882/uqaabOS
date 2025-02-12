#include "../include/syscall.h"

namespace uqaabOS {
namespace syscall {

// Constructor for SyscallHandler, initializes the interrupt handler with the given interrupt number
SyscallHandler::SyscallHandler(interrupts::InterruptManager *interrupt_manager, uint8_t interrupt_number)
    : InterruptHandler(interrupt_manager, interrupt_number + interrupt_manager->hardwareInterruptOffset()) {}

// Destructor for SyscallHandler
SyscallHandler::~SyscallHandler() {}

/**
 * Handles the system call interrupt.
 * esp: The stack pointer at the time of the interrupt.
 * returns The stack pointer to be restored after handling the interrupt.
 */
uint32_t SyscallHandler::handle_interrupt(uint32_t esp) {
  multitasking::CPUState *cpu = (multitasking::CPUState *)esp;

  // Check the value in the eax register to determine which system call to handle
  switch (cpu->eax) {
  case 4:
    // System call to print a string (pointer in ebx)
    libc::printf((char *)cpu->ebx);
    break;

  default:
    // Handle other system calls here
    break;
  }

  return esp; // Return the stack pointer to be restored
}

} // namespace syscall
} // namespace uqaabOS
