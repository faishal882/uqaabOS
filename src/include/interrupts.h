#ifndef __INTERRUPTMANAGER_H
#define __INTERRUPTMANAGER_H

#include <stdint.h>

#include "gdt.h"
#include "libc/stdio.h"
#include "port.h"

namespace uqaabOS {
namespace interrupts {

extern "C" {
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

// Exceptions(eg: 0x00: Divide error Interrupt, 0x01: Debug Interrupt,
// , 0x06: Invalid Opcode Interrupt etc)
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

// a generic ISR(interrupt service routine) handler
uint32_t handle_interrupt(uint8_t interrupt_number, uint32_t esp);

// dummy ISR for unused interrupts.
static void interrupt_ignore();
}

/*  **** Gate Descriptor(Interrupt Table Entry) ****
   -> low_offset(0-15bits): Lower 16 bits of the ISR address
   -> code_seg_selector(16-31bits): Selector of the code segment
   -> zero(32-39bits): always 0(Reserved)
   -> type(40-47bits): (40-43)Gate type(Task gate, Interrupt gate etc),
                      (44)0 always, (45-46)DPL(CPU Privilege level),
                      (47)Present(1 for valid entry)
   -> high_offset(36-63bits): Higher 16 bits of the ISR address
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
public:
  /* 0x00-0xFF(255): Entries in the IDT
     -> Trap Gate(0x00-0x1F): CPU Exceptions (Faults, Traps),
     -> Interrupt Gate(0x20-0x2F): IRQ 0–15 (Hardware Interrupts),
     -> Task Gate(0x30-0xFF): Reserved / Custom Software Interrupts
   */
  // static struct GateDescriptor idt_entries[256]; // IDT entries

  // set gate descriptor (set the interrupt table entry)
  static void setGateDescriptor(uint8_t interrupt,
                                uint16_t code_seg_selector_offset,
                                void (*handler)(), uint8_t desc_privilege_level,
                                uint8_t desc_type);

  // hardware interrupt offset(for resolving conflict with CPU exceptions in
  // IDT, i.e: PIC maps IRQs 0–15 to IDT entries 0x08–0x0F conflict with CPU
  // exceptions, eg: IRQ0x00->0x08 Original IDT entry->Remapped entry 0x20 etc)
  uint16_t hardware_interrupt_offset;

  // Controls Intel 8259 PIC,routes hardware interrupts to CPU.
  // master-Programmable Interrupt Controller command port: 0x0020,
  // master-PIC data port: 0x0021
  // slave-Programmable Interrupt Controller command port: 0x00A0,
  // slave-PIC data port: 0x00A1
  uqaabOS::include::Port8BitSlow PIC_master_command_port;
  uqaabOS::include::Port8BitSlow PIC_master_data_port;
  uqaabOS::include::Port8BitSlow PIC_slave_command_port;
  uqaabOS::include::Port8BitSlow PIC_slave_data_port;

public:
  InterruptManager(uint16_t hardware_interrupt_offset,
                   uqaabOS::include::GDT *gdt);
  ~InterruptManager();

  uint16_t hardwareInterruptOffset();
  void activate();   // activate the interrupts
  void deactivate(); // deactivate the interrupts
};
} // namespace interrupts
} // namespace uqaabOS

#endif
