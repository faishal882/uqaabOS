#include <stdint.h>

#include "../include/keyboard/keyboard.h"
#include "../include/libc/stdio.h"

namespace uqaabOS
{

    namespace keyboard
    {
        /*
          -> 0x21 is the IRQ (Interrupt Request Line) for the keyboard.
          -> data_port to 0x60, I/O port used to communicate with the keyboard for reading and writing data.
          -> command_port 0x64, I/O port for sending commands to the keyboard controller and reading its status.
        */

        KeyBoardDriver::KeyBoardDriver(uqaabOS::interrupts::InterruptManager *interrupt_manager)
            : uqaabOS::interrupts::InterruptHandler(interrupt_manager, 0x21),
              data_port(0x60),
              command_port(0x64)
        {

            /*keyboard controller may have pending data in its input buffer (status register bit 0 set to 1).
            -> status register via the command_port the input buffer is full 
            */
            while (command_port.read() && 0x1)
            {
                // read to clear the buffer,
                data_port.read();
            }

            // command 0xae = Activates the keyboard interrupt mechanism.
            command_port.write(0xae);

            // command 0x20 = read controller command byte
            command_port.write(0x20);

            /*
            -> Ensures the keyboard interrupt (bit 0) is enabled.
            -> Disables the keyboard clock (bit 4) to prevent unintended behavior during initialization.
            */
            uint8_t status = (data_port.read() | 1) & ~0x10;

            /*0x60: Writes the updated command byte to the keyboard controller.*/
            command_port.write(0x60);
            data_port.write(status);

            /*0xF4: Enables scanning on the keyboard.*/
            data_port.write(0xf4);
        }

        KeyBoardDriver::~KeyBoardDriver() {}

        uint32_t KeyBoardDriver::handle_interrupt(uint32_t esp)
        {

            uint8_t key = data_port.read();

            /*key is press -> the keyboard sends a scancode that is a single byte,(less tha  128)*/
            if (key < 0x80)
            {

                switch (key)
                {
                case 0x02:
                    uqaabOS::libc::printf("1");
                    break;
                case 0x03:
                    uqaabOS::libc::printf("2");
                    break;
                case 0x04:
                    uqaabOS::libc::printf("3");
                    break;
                case 0x05:
                    uqaabOS::libc::printf("4");
                    break;
                case 0x06:
                    uqaabOS::libc::printf("5");
                    break;
                case 0x07:
                    uqaabOS::libc::printf("6");
                    break;
                case 0x08:
                    uqaabOS::libc::printf("7");
                    break;
                case 0x09:
                    uqaabOS::libc::printf("8");
                    break;
                case 0x0A:
                    uqaabOS::libc::printf("9");
                    break;
                case 0x0B:
                    uqaabOS::libc::printf("0");
                    break;

                case 0x10:
                    uqaabOS::libc::printf("q");
                    break;
                case 0x11:
                    uqaabOS::libc::printf("w");
                    break;
                case 0x12:
                    uqaabOS::libc::printf("e");
                    break;
                case 0x13:
                    uqaabOS::libc::printf("r");
                    break;
                case 0x14:
                    uqaabOS::libc::printf("t");
                    break;
                case 0x15:
                    uqaabOS::libc::printf("z");
                    break;
                case 0x16:
                    uqaabOS::libc::printf("u");
                    break;
                case 0x17:
                    uqaabOS::libc::printf("i");
                    break;
                case 0x18:
                    uqaabOS::libc::printf("o");
                    break;
                case 0x19:
                    uqaabOS::libc::printf("p");
                    break;

                case 0x1E:
                    uqaabOS::libc::printf("a");
                    break;
                case 0x1F:
                    uqaabOS::libc::printf("s");
                    break;
                case 0x20:
                    uqaabOS::libc::printf("d");
                    break;
                case 0x21:
                    uqaabOS::libc::printf("f");
                    break;
                case 0x22:
                    uqaabOS::libc::printf("g");
                    break;
                case 0x23:
                    uqaabOS::libc::printf("h");
                    break;
                case 0x24:
                    uqaabOS::libc::printf("j");
                    break;
                case 0x25:
                    uqaabOS::libc::printf("k");
                    break;
                case 0x26:
                    uqaabOS::libc::printf("l");
                    break;
                case 0x2C:
                    uqaabOS::libc::printf("y");
                    break;
                case 0x2D:
                    uqaabOS::libc::printf("x");
                    break;
                case 0x2E:
                    uqaabOS::libc::printf("c");
                    break;
                case 0x2F:
                    uqaabOS::libc::printf("v");
                    break;
                case 0x30:
                    uqaabOS::libc::printf("b");
                    break;
                case 0x31:
                    uqaabOS::libc::printf("n");
                    break;
                case 0x32:
                    uqaabOS::libc::printf("m");
                    break;
                case 0x33:
                    uqaabOS::libc::printf(",");
                    break;
                case 0x34:
                    uqaabOS::libc::printf(".");
                    break;
                case 0x35:
                    uqaabOS::libc::printf("-");
                    break;

                case 0x1C:
                    uqaabOS::libc::printf("\n");
                    break;
                case 0x39:
                    uqaabOS::libc::printf(" ");
                    break;

                default:
                    char foo[] = "KEYBOARD 0x00 ";
                    char hex[] = "0123456789ABCDEF";
                    foo[11] = hex[(key >> 4) & 0xF];
                    foo[12] = hex[key & 0xF];
                    uqaabOS::libc::printf(foo);
                    break;
                }
            }

            return esp;
        }
    }
}