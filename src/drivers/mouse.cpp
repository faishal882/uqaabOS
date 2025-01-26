// Include the mouse driver header file
#include "../include/drivers/mouse.h"

namespace uqaabOS {
namespace driver {

// Base class for handling mouse events
MouseEventHandler::MouseEventHandler() {}

// Called when mouse is activated
void MouseEventHandler::on_activate() {}
// Called when a mouse button is pressed
void MouseEventHandler::on_mouse_down(uint8_t button) {}
// Called when a mouse button is released
void MouseEventHandler::on_mouse_up(uint8_t button) {}
// Called when mouse cursor moves
void MouseEventHandler::on_mouse_move(int x, int y) {}

// Takes interrupt manager and event handler as parameters
MouseDriver::MouseDriver(interrupts::InterruptManager *manager,
                         MouseEventHandler *handler)
    : InterruptHandler(manager, 0x2C), // Initialize with IRQ 0x2C for mouse
      data_port(0x60),                 // Port for mouse data
      command_port(0x64) {             // Port for mouse commands
  this->handler = handler;             // Store the event handler
}

// Destructor
MouseDriver::~MouseDriver() {}

// Activate the mouse driver
void MouseDriver::activate() {
  // Initialize mouse state
  offset = 0;  // Reset buffer offset
  buttons = 0; // Reset button states

  // If handler exists, call its activation method
  if (handler != 0)
    handler->on_activate();

  // Mouse initialization sequence
  // 0xA8 command: Enables the second PS/2 port (where mouse is connected)
  command_port.write(0xA8);
  command_port.write(0x20);              // Get controller command byte
  uint8_t status = data_port.read() | 2; // Set bit 1 to enable interrupts
  command_port.write(0x60);              // Set controller command byte
  data_port.write(status);               // Write modified status back

  command_port.write(0xD4); // Tell controller to forward next byte to mouse
  /* 0xF4 command to mouse:
   - Enables movement data transmission
   - Enables button press reporting
   - Mouse will now send data packets when moved/clicked */
  data_port.write(0xF4);
  /* Mouse acknowledgment:
   - 0xFA means command accepted
   - If any other value, initialization might have failed */
  data_port.read(); // Read acknowledgment byte
}

// Handle mouse interrupts
uint32_t MouseDriver::handle_interrupt(uint32_t esp) {
  // Read mouse controller status
  uint8_t status = command_port.read();
  // Return if no mouse data available
  if (!(status & 0x20))
    return esp;

  // Read the mouse data byte
  buffer[offset] = data_port.read();

  // Return if no handler registered
  if (handler == 0)
    return esp;

  /*  implements a circular buffer for mouse data. (3 bytes per packet)
      -> byte 0: button states,
      -> byte 1: X movement,
      -> byte 2: Y movement
  */
  offset = (offset + 1) % 3;

  // Process complete mouse packet (3 bytes received)
  if (offset == 0) {
    // Handle mouse movement if any
    if (buffer[1] != 0 || buffer[2] != 0) {
      // Call movement handler (note: Y axis is inverted)
      // handler->on_mouse_move(buffer[1], -buffer[2]);
      handler->on_mouse_move((int8_t)buffer[1], -((int8_t)buffer[2]));
    }

    // Check for button state changes
    for (uint8_t i = 0; i < 3; i++) {
      // Compare current button state with previous
      if ((buffer[0] & (0x1 << i)) != (buttons & (0x1 << i))) {
        // If button was previously pressed, notify button down
        if (buttons & (0x1 << i))
          handler->on_mouse_down(i + 1);
      }
    }
    // Store current button state for next comparison
    buttons = buffer[0];
  }

  return esp; // Return unchanged stack pointer
}

} // namespace driver
} // namespace uqaabOS