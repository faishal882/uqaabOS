#ifndef __SYSCALLS_H 
#define __SYSCALLS_H

#include "./interrupts.h"
#include "./libc/stdio.h" 
#include "./multitasking/multitasking.h" 
#include <stdint.h> 

namespace uqaabOS { 
namespace syscall { 

// SyscallHandler class handles system calls by extending the InterruptHandler class
class SyscallHandler : public interrupts::InterruptHandler {

public:
    // Constructor for SyscallHandler, takes an InterruptManager pointer and an interrupt number
    SyscallHandler(interrupts::InterruptManager *interrupt_manager,
                                 uint8_t Interrupt_number);
    // Destructor for SyscallHandler
    ~SyscallHandler();

    // Method to handle the interrupt, takes the stack pointer as an argument and returns a new stack pointer
    virtual uint32_t handle_interrupt(uint32_t esp);
};
} // namespace syscall
} // namespace uqaabOS

#endif 