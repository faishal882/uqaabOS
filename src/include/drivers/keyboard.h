#ifndef __KEYBOARD_H
#define __KEYBOARD_H

#include <stdint.h>

#include "../interrupts.h"
#include "../libc/stdio.h"
#include "../port.h"

namespace uqaabOS
{

    namespace driver
    {
        class KeyboardDriver : public interrupts::InterruptHandler
        {
        private:
            /*
             -> keyboard use 8 bit registers for communication
            */
            /*data port is used to Read data from the keyboard */
            include::Port8Bit data_port;

            /*send commands to the keyboard controller or read its status.
            -> 0xFF: Reset the keyboard.
            -> 0xF0: Set scancode set.
            -> 0xFA: Acknowledge (command was successful)
            */
            include::Port8Bit command_port;

        public:
            KeyboardDriver(interrupts::InterruptManager *manager);
            ~KeyboardDriver();
            virtual uint32_t  handle_interrupt(uint32_t esp);
        };

    }
}
#endif