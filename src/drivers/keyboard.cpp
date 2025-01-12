
#include "../include/drivers/keyboard.h"

namespace uqaabOS {
namespace driver {
KeyboardDriver::KeyboardDriver(interrupts::InterruptManager *manager)
    : interrupts::InterruptHandler(manager, 0x21), data_port(0x60),
      command_port(0x64) {
  while (command_port.read() & 0x1)
    data_port.read();
  command_port.write(0xae); // activate interrupts
  command_port.write(0x20); // command 0x20 = read controller command byte
  uint8_t status = (data_port.read() | 1) & ~0x10;
  command_port.write(0x60); // command 0x60 = set controller command byte
  data_port.write(status);
  data_port.write(0xf4);
}

KeyboardDriver::~KeyboardDriver() {}

uint32_t KeyboardDriver::handle_interrupt(uint32_t esp) {
  uint8_t key = data_port.read();
  if (key < 0x80) {
    switch (key) {
    case 0x02:
      libc::printf("1");
      break;
    case 0x03:
      libc::printf("2");
      break;
    case 0x04:
      libc::printf("3");
      break;
    case 0x05:
      libc::printf("4");
      break;
    case 0x06:
      libc::printf("5");
      break;
    case 0x07:
      libc::printf("6");
      break;
    case 0x08:
      libc::printf("7");
      break;
    case 0x09:
      libc::printf("8");
      break;
    case 0x0A:
      libc::printf("9");
      break;
    case 0x0B:
      libc::printf("0");
      break;

    case 0x10:
      libc::printf("q");
      break;
    case 0x11:
      libc::printf("w");
      break;
    case 0x12:
      libc::printf("e");
      break;
    case 0x13:
      libc::printf("r");
      break;
    case 0x14:
      libc::printf("t");
      break;
    case 0x15:
      libc::printf("z");
      break;
    case 0x16:
      libc::printf("u");
      break;
    case 0x17:
      libc::printf("i");
      break;
    case 0x18:
      libc::printf("o");
      break;
    case 0x19:
      libc::printf("p");
      break;

    case 0x1E:
      libc::printf("a");
      break;
    case 0x1F:
      libc::printf("s");
      break;
    case 0x20:
      libc::printf("d");
      break;
    case 0x21:
      libc::printf("f");
      break;
    case 0x22:
      libc::printf("g");
      break;
    case 0x23:
      libc::printf("h");
      break;
    case 0x24:
      libc::printf("j");
      break;
    case 0x25:
      libc::printf("k");
      break;
    case 0x26:
      libc::printf("l");
      break;

    case 0x2C:
      libc::printf("y");
      break;
    case 0x2D:
      libc::printf("x");
      break;
    case 0x2E:
      libc::printf("c");
      break;
    case 0x2F:
      libc::printf("v");
      break;
    case 0x30:
      libc::printf("b");
      break;
    case 0x31:
      libc::printf("n");
      break;
    case 0x32:
      libc::printf("m");
      break;
    case 0x33:
      libc::printf(",");
      break;
    case 0x34:
      libc::printf(".");
      break;
    case 0x35:
      libc::printf("-");
      break;

    case 0x1C:
      libc::printf("\n");
      break;
    case 0x39:
      libc::printf(" ");
      break;

    default: {
      char *foo = "KEYBOARD 0x00 ";
      char *hex = "0123456789ABCDEF";
      foo[11] = hex[(key >> 4) & 0xF];
      foo[12] = hex[key & 0xF];
      libc::printf(foo);
      break;
    }
    }
  }
  return esp;
}
} // namespace driver
} // namespace uqaabOS
