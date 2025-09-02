#include "../include/terminal/terminal_keyboard.h"

namespace uqaabOS {
namespace terminal {

void TerminalKeyboardEventHandler::on_key_down(char c) {
  if (terminal != nullptr) {
    terminal->handle_key_press(c);
  }
}

void TerminalKeyboardEventHandler::on_special_key_down(uint8_t scancode) {
  // Handle special keys if needed
  // For now, we're handling arrow keys directly in the driver
}

} // namespace terminal
} // namespace uqaabOS