#ifndef __TERMINAL_KEYBOARD_H
#define __TERMINAL_KEYBOARD_H

#include "../drivers/keyboard.h"
#include "terminal.h"

namespace uqaabOS {
namespace terminal {

class TerminalKeyboardEventHandler
    : public driver::KeyboardEventHandler {
private:
    Terminal* terminal;
    
public:
    TerminalKeyboardEventHandler(Terminal* term = nullptr) : terminal(term) {}
    
    void set_terminal(Terminal* term) { terminal = term; }
    void on_key_down(char c) override;
    void on_special_key_down(uint8_t scancode) override;
};

} // namespace terminal
} // namespace uqaabOS

#endif // __TERMINAL_KEYBOARD_H