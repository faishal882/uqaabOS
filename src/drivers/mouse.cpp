#include "../include/drivers/mouse.h"

namespace uqaabOS {
namespace driver {
MouseDriver::MouseDriver(interrupts::InterruptManager *manager)
    : InterruptHandler(manager, 0x2C), data_port(0x60), command_port(0x64) {
  uint16_t *VideoMemory = (uint16_t *)0xb8000;
  offset = 0;
  buttons = 0;
  x = 40;
  y = 12;
  VideoMemory[80 * y + x] = (VideoMemory[80 * y + x] & 0x0F00) << 4 |
                            (VideoMemory[80 * y + x] & 0xF000) >> 4 |
                            (VideoMemory[80 * y + x] & 0x00FF);
  command_port.write(0xA8);
  command_port.write(0x20); // command 0x60 = read controller command byte
  uint8_t status = data_port.read() | 2;
  command_port.write(0x60); // command 0x60 = set controller command byte
  data_port.write(status);
  command_port.write(0xD4);
  data_port.write(0xF4);
  data_port.read();
}
MouseDriver::~MouseDriver() {}

uint32_t MouseDriver::handle_interrupt(uint32_t esp) {
  uint8_t status = command_port.read();
  if (!(status & 0x20))
    return esp;
  buffer[offset] = data_port.read();
  offset = (offset + 1) % 3;
  if (offset == 0) {
    if (buffer[1] != 0 || buffer[2] != 0) {
      static uint16_t *VideoMemory = (uint16_t *)0xb8000;
      VideoMemory[80 * y + x] = (VideoMemory[80 * y + x] & 0x0F00) << 4 |
                                (VideoMemory[80 * y + x] & 0xF000) >> 4 |
                                (VideoMemory[80 * y + x] & 0x00FF);
      x += buffer[1];
      if (x >= 80)
        x = 79;
      if (x < 0)
        x = 0;
      y -= buffer[2];
      if (y >= 25)
        y = 24;
      if (y < 0)
        y = 0;
      VideoMemory[80 * y + x] = (VideoMemory[80 * y + x] & 0x0F00) << 4 |
                                (VideoMemory[80 * y + x] & 0xF000) >> 4 |
                                (VideoMemory[80 * y + x] & 0x00FF);
    }
  }
  return esp;
}
} // namespace driver
} // namespace uqaabOS
