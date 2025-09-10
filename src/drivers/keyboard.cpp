#include <stdint.h>

#include "../include/drivers/keyboard.h"

namespace uqaabOS {

namespace driver {

KeyboardEventHandler::KeyboardEventHandler() {}
void KeyboardEventHandler::on_key_down(char) {}
void KeyboardEventHandler::on_key_up(char) {}
void KeyboardEventHandler::on_special_key_down(uint8_t) {}
/*
  -> 0x21 is the IRQ (Interrupt Request Line) for the keyboard.
  -> data_port to 0x60, I/O port used to communicate with the keyboard for
  reading and writing data.
  -> command_port 0x64, I/O port for sending commands to the keyboard controller
  and reading its status.
*/

KeyboardDriver::KeyboardDriver(interrupts::InterruptManager *manager,
                               KeyboardEventHandler *handler)
    : interrupts::InterruptHandler(manager, 0x21), data_port(0x60),
      command_port(0x64) {

  this->handler = handler;
}

void KeyboardDriver::activate() {
  /*keyboard controller may have pending data in its input buffer (status
 register bit 0 set to 1).
 -> status register via the command_port the input buffer is full
 */
  while (command_port.read() & 0x1)
    data_port.read();
  // read to clear the buffer,

  // command 0xae = Activates the keyboard interrupt mechanism.
  command_port.write(0xae);

  // command 0x20 = read controller command byte
  command_port.write(0x20);

  /*
  -> Ensures the keyboard interrupt (bit 0) is enabled.
  -> Disables the keyboard clock (bit 4) to prevent unintended behavior during
  initialization.
  */
  uint8_t status = (data_port.read() | 1) & ~0x10;

  /*0x60: Writes the updated command byte to the keyboard controller.*/
  command_port.write(0x60);
  data_port.write(status);

  /*0xF4: Enables scanning on the keyboard.*/
  data_port.write(0xf4);
}

KeyboardDriver::~KeyboardDriver() {}

uint32_t KeyboardDriver::handle_interrupt(uint32_t esp) {
  uint8_t key = data_port.read();
  
  // Check if it's a key release (bit 7 set)
  bool released = key & 0x80;
  uint8_t scancode = key & 0x7F; // Clear the release bit
  
  if (!released) {
    // Handle special keys (arrow keys, function keys, etc.)
    switch (scancode) {
    // Arrow keys (using scancode set 1)
    case 0x48: // Up arrow
    case 0x50: // Down arrow
    case 0x4B: // Left arrow
    case 0x4D: // Right arrow
      handler->on_special_key_down(scancode);
      uqaabOS::libc::move_cursor(
          (scancode == 0x4D) - (scancode == 0x4B),  // dx: +1 for right, -1 for left
          (scancode == 0x50) - (scancode == 0x48)   // dy: +1 for down, -1 for up
      );
      break;
      
    // Regular character keys
    case 0x02:
      handler->on_key_down('1');
      break;
    case 0x03:
      handler->on_key_down('2');
      break;
    case 0x04:
      handler->on_key_down('3');
      break;
    case 0x05:
      handler->on_key_down('4');
      break;
    case 0x06:
      handler->on_key_down('5');
      break;
    case 0x07:
      handler->on_key_down('6');
      break;
    case 0x08:
      handler->on_key_down('7');
      break;
    case 0x09:
      handler->on_key_down('8');
      break;
    case 0x0A:
      handler->on_key_down('9');
      break;
    case 0x0B:
      handler->on_key_down('0');
      break;

    case 0x10:
      handler->on_key_down('q');
      break;
    case 0x11:
      handler->on_key_down('w');
      break;
    case 0x12:
      handler->on_key_down('e');
      break;
    case 0x13:
      handler->on_key_down('r');
      break;
    case 0x14:
      handler->on_key_down('t');
      break;
    case 0x15:
      handler->on_key_down('z');
      break;
    case 0x16:
      handler->on_key_down('u');
      break;
    case 0x17:
      handler->on_key_down('i');
      break;
    case 0x18:
      handler->on_key_down('o');
      break;
    case 0x19:
      handler->on_key_down('p');
      break;

    case 0x1E:
      handler->on_key_down('a');
      break;
    case 0x1F:
      handler->on_key_down('s');
      break;
    case 0x20:
      handler->on_key_down('d');
      break;
    case 0x21:
      handler->on_key_down('f');
      break;
    case 0x22:
      handler->on_key_down('g');
      break;
    case 0x23:
      handler->on_key_down('h');
      break;
    case 0x24:
      handler->on_key_down('j');
      break;
    case 0x25:
      handler->on_key_down('k');
      break;
    case 0x26:
      handler->on_key_down('l');
      break;

    case 0x2C:
      handler->on_key_down('y');
      break;
    case 0x2D:
      handler->on_key_down('x');
      break;
    case 0x2E:
      handler->on_key_down('c');
      break;
    case 0x2F:
      handler->on_key_down('v');
      break;
    case 0x30:
      handler->on_key_down('b');
      break;
    case 0x31:
      handler->on_key_down('n');
      break;
    case 0x32:
      handler->on_key_down('m');
      break;
    case 0x33:
      handler->on_key_down(',');
      break;
    case 0x34:
      handler->on_key_down('.');
      break;
    case 0x35:
      handler->on_key_down('/');
      break;

    case 0x1C:
      handler->on_key_down('\n');
      break;
    case 0x0E: 
      handler->on_key_down('\b'); 
      break;
    case 0x39:
      handler->on_key_down(' ');
      break;

    default: {
      uqaabOS::libc::printf("KEYBOARD 0x");
      uqaabOS::libc::print_hex(scancode);
      break;
    }
    }
  }
  return esp;
}
} // namespace driver
} // namespace uqaabOS