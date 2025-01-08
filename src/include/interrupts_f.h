#ifndef __INTERRUPTMANAGER_H
#define __INTERRUPTMANAGER_H

#include <stdint.h>

namespace uqaabOS {
namespace interrupts {

/*  **** Gate Descriptor(Interrupt Table Entry) ****
   -> low_offset(0-15bytes): Lower 16 bits of the ISR address
   -> code_seg_selector(16-31bytes): Selector of the code segment
   -> zero(32-39bytes): always 0(Reserved)
   -> type(40-47bytes): type of the interrupt and also privilege level
   -> high_offset(36-63bytes): Higher 16 bits of the ISR address
   */
struct GateDescriptor {
  uint16_t low_offset;
  uint16_t code_seg_selector;
  uint8_t zero;
  uint8_t type;
  uint16_t high_offset;
} __attribute__((packed));

/*  **** IDTPointer(Interrupt Descriptor Table Pointer) ****
   -> size: size of the IDT
   -> base: base address of the IDT
   */
struct IDTPointer {
  uint16_t size;
  uint32_t base;
} __attribute__((packed));

class InterruptManager {
protected:
  static struct GateDescriptor idt_entries[256]; // IDT entries

  // set gate descriptor (set the interrupt table entry)
  static void setGateDescriptor(uint8_t interrupt,
                                uint16_t code_seg_selector_offset,
                                void (*handler)(), uint8_t desc_privilege_level,
                                uint8_t desc_type);

  // dummy ISR for unused interrupts.
  static void InterruptIgnore();

  // Interrupt Requests(eg: 0x00: Programmable interrupt timer Interrupt
  // , 0x01: keyboard interrupt etc)
  static void IRQ0x00();
  static void IRQ0x01();
  static void IRQ0x02();
  static void IRQ0x03();
  static void IRQ0x04();
  static void IRQ0x05();
  static void IRQ0x06();
  static void IRQ0x07();
  static void IRQ0x08();
  static void IRQ0x09();
  static void IRQ0x0A();
  static void IRQ0x0B();
  static void IRQ0x0C();
  static void IRQ0x0D();
  static void IRQ0x0E();
  static void IRQ0x0F();
  static void IRQ0x31();

  static void handle_exception0x00();
  static void handle_exception0x01();
  static void handle_exception0x02();
  static void handle_exception0x03();
  static void handle_exception0x04();
  static void handle_exception0x05();
  static void handle_exception0x06();
  static void handle_exception0x07();
  static void handle_exception0x08();
  static void handle_exception0x09();
  static void handle_exception0x0A();
  static void handle_exception0x0B();
  static void handle_exception0x0C();
  static void handle_exception0x0D();
  static void handle_exception0x0E();
  static void handle_exception0x0F();
  static void handle_exception0x10();
  static void handle_exception0x11();
  static void handle_exception0x12();
  static void handle_exception0x13();

  uint32_t handle_interrupt(uint8_t interrupt_number, uint32_t esp);

public:
  InterruptManager();
  ~InterruptManager();
};
} // namespace interrupts
} // namespace uqaabOS

#endif
