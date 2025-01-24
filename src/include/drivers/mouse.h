#ifndef __MOUSE_H
#define __MOUSE_H

#include "../interrupts.h"
#include "../port.h"
#include "driver.h"

namespace uqaabOS {
namespace driver {

class MouseEventHandler {
public:
  MouseEventHandler();

  virtual void on_activate();
  virtual void on_mouse_down(uint8_t button);
  virtual void on_mouse_up(uint8_t button);
  virtual void on_mouse_move(int x, int y);
};

class MouseDriver : public interrupts::InterruptHandler, public Driver {
  include::Port8Bit data_port;
  include::Port8Bit command_port;
  uint8_t buffer[3];
  uint8_t offset;
  uint8_t buttons;

  MouseEventHandler *handler;

public:
  MouseDriver(interrupts::InterruptManager *manager,
              MouseEventHandler *handler);
  ~MouseDriver();
  virtual uint32_t handle_interrupt(uint32_t esp);
  virtual void activate();
};
} // namespace driver
} // namespace uqaabOS
#endif
