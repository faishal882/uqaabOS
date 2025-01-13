#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include <stdint.h>

#include "../interrupts.h";
#include "../port.h"

namespace uqaabOS
{

    namespace keyboard
    {
        class KeyBoardDriver : public uqaabOS::interrupts::InterruptHandler
        {
        private:
            /*
             -> keyboard use 8 bit registers for communication
            */
            /*data port is used to Read data from the keyboard */
            uqaabOS::include::Port8Bit data_port;

            /*send commands to the keyboard controller or read its status.
            -> 0xFF: Reset the keyboard.
            -> 0xF0: Set scancode set.
            -> 0xFA: Acknowledge (command was successful)
            */
            uqaabOS::include::Port8Bit command_port;

        public:
            KeyBoardDriver(uqaabOS::interrupts::InterruptManager *interrupt_manager);
            ~KeyBoardDriver();
            virtual uint32_t handle_interrupt(uint32_t esp);
        };

    }
}
#endif