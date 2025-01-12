#ifndef __KEYBOARD_H
#define __KEYBOARD_H

#include "../interrupts.h"
#include "../libc/stdio.h"
#include "../port.h"

namespace uqaabOS {
namespace driver {
class KeyboardDriver : public interrupts::InterruptHandler {
  include::Port8Bit data_port;
  include::Port8Bit command_port;

public:
  KeyboardDriver(interrupts::InterruptManager *manager);
  ~KeyboardDriver();
  virtual uint32_t handle_interrupt(uint32_t esp);
};
} // namespace driver
} // namespace uqaabOS

#endif
