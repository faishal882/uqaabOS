#ifndef __MOUSE_H
#define __MOUSE_H

#include "../interrupts.h"
#include "../port.h"

namespace uqaabOS {
namespace driver {

class MouseDriver : public interrupts::InterruptHandler {
  include::Port8Bit data_port;
  include::Port8Bit command_port;
  uint8_t buffer[3];
  uint8_t offset;
  uint8_t buttons;
  int8_t x, y;

public:
  MouseDriver(interrupts::InterruptManager *manager);
  ~MouseDriver();
  virtual uint32_t handle_interrupt(uint32_t esp);
};
} // namespace driver
} // namespace uqaabOS
#endif
