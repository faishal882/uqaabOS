#include "../../include/interrupts.h"

namespace uqaabOS {
namespace interrupts {

/* 0x00-0xFF(255): Entries in the IDT
   -> Trap Gate(0x00-0x1F): CPU Exceptions (Faults, Traps),
   -> Interrupt Gate(0x20-0x2F): IRQ 0–15 (Hardware Interrupts),
   -> Task Gate(0x30-0xFF): Reserved / Custom Software Interrupts
 */
static struct GateDescriptor idt_entries[256]; // IDT entries

// set gate descriptor (set the interrupt table entry)
void InterruptManager::setGateDescriptor(uint8_t interrupt,
                                         uint16_t code_seg_selector_offset,
                                         void (*handler)(),
                                         uint8_t desc_privilege_level,
                                         uint8_t desc_type) {
  // set ISR(Interrupt Service Routine)
  idt_entries[interrupt].low_offset = ((uint32_t)handler) & 0xFFFF;
  idt_entries[interrupt].high_offset = (((uint32_t)handler) >> 16) & 0xFFFF;

  // set code segment of the ISR
  idt_entries[interrupt].code_seg_selector = code_seg_selector_offset;
  idt_entries[interrupt].zero = 0;

  // set privilege level
  const uint8_t IDT_DESC_PRESENT = 0x80; // binary: 1000 0000
  idt_entries[interrupt].type =
      IDT_DESC_PRESENT | ((desc_privilege_level & 3) << 5) | (desc_type & 0xF);
}

// setup interrupt manager
InterruptManager::InterruptManager(uint16_t hardware_interrupt_offset,
                                   uqaabOS::include::GDT *gdt)
    : PIC_master_command_port(0x20), PIC_master_data_port(0x21),
      PIC_slave_command_port(0xA0), PIC_slave_data_port(0xA1) {
  this->hardware_interrupt_offset = hardware_interrupt_offset;
  // ISR code segment
  uint32_t code_segment = gdt->code_segment_selector();

  // Gate Type: 32-bit Interrupt Gate: 0xE
  const uint8_t IDT_INTERRUPT_GATE = 0xE;
  for (uint8_t i = 255; i > 0; --i) {
    setGateDescriptor(i, code_segment, &interrupt_ignore, 0,
                      IDT_INTERRUPT_GATE);
  }
  setGateDescriptor(0, code_segment, &interrupt_ignore, 0, IDT_INTERRUPT_GATE);

  // hardware interrupts, Interrupt Gate
  setGateDescriptor(hardware_interrupt_offset + 0x00, code_segment, &IRQ0x00, 0,
                    IDT_INTERRUPT_GATE);
  setGateDescriptor(hardware_interrupt_offset + 0x01, code_segment, &IRQ0x01, 0,
                    IDT_INTERRUPT_GATE);
  setGateDescriptor(hardware_interrupt_offset + 0x02, code_segment, &IRQ0x02, 0,
                    IDT_INTERRUPT_GATE);
  setGateDescriptor(hardware_interrupt_offset + 0x03, code_segment, &IRQ0x03, 0,
                    IDT_INTERRUPT_GATE);
  setGateDescriptor(hardware_interrupt_offset + 0x04, code_segment, &IRQ0x04, 0,
                    IDT_INTERRUPT_GATE);
  setGateDescriptor(hardware_interrupt_offset + 0x05, code_segment, &IRQ0x05, 0,
                    IDT_INTERRUPT_GATE);
  setGateDescriptor(hardware_interrupt_offset + 0x06, code_segment, &IRQ0x06, 0,
                    IDT_INTERRUPT_GATE);
  setGateDescriptor(hardware_interrupt_offset + 0x07, code_segment, &IRQ0x07, 0,
                    IDT_INTERRUPT_GATE);
  setGateDescriptor(hardware_interrupt_offset + 0x08, code_segment, &IRQ0x08, 0,
                    IDT_INTERRUPT_GATE);
  setGateDescriptor(hardware_interrupt_offset + 0x09, code_segment, &IRQ0x09, 0,
                    IDT_INTERRUPT_GATE);
  setGateDescriptor(hardware_interrupt_offset + 0x0A, code_segment, &IRQ0x0A, 0,
                    IDT_INTERRUPT_GATE);
  setGateDescriptor(hardware_interrupt_offset + 0x0B, code_segment, &IRQ0x0B, 0,
                    IDT_INTERRUPT_GATE);
  setGateDescriptor(hardware_interrupt_offset + 0x0C, code_segment, &IRQ0x0C, 0,
                    IDT_INTERRUPT_GATE);
  setGateDescriptor(hardware_interrupt_offset + 0x0D, code_segment, &IRQ0x0D, 0,
                    IDT_INTERRUPT_GATE);
  setGateDescriptor(hardware_interrupt_offset + 0x0E, code_segment, &IRQ0x0E, 0,
                    IDT_INTERRUPT_GATE);
  setGateDescriptor(hardware_interrupt_offset + 0x0F, code_segment, &IRQ0x0F, 0,
                    IDT_INTERRUPT_GATE);

  // handle exceptions(Trap Gate)
  const uint8_t IDT_TRAP_GATE = 0xF;
  setGateDescriptor(0x00, code_segment, &handle_exception0x00, 0,
                    IDT_TRAP_GATE);
  setGateDescriptor(0x01, code_segment, &handle_exception0x01, 0,
                    IDT_TRAP_GATE);
  setGateDescriptor(0x02, code_segment, &handle_exception0x02, 0,
                    IDT_TRAP_GATE);
  setGateDescriptor(0x03, code_segment, &handle_exception0x03, 0,
                    IDT_TRAP_GATE);
  setGateDescriptor(0x04, code_segment, &handle_exception0x04, 0,
                    IDT_TRAP_GATE);
  setGateDescriptor(0x05, code_segment, &handle_exception0x05, 0,
                    IDT_TRAP_GATE);
  setGateDescriptor(0x06, code_segment, &handle_exception0x06, 0,
                    IDT_TRAP_GATE);
  setGateDescriptor(0x07, code_segment, &handle_exception0x07, 0,
                    IDT_TRAP_GATE);
  setGateDescriptor(0x08, code_segment, &handle_exception0x08, 0,
                    IDT_TRAP_GATE);
  setGateDescriptor(0x09, code_segment, &handle_exception0x09, 0,
                    IDT_TRAP_GATE);
  setGateDescriptor(0x0A, code_segment, &handle_exception0x0A, 0,
                    IDT_TRAP_GATE);
  setGateDescriptor(0x0B, code_segment, &handle_exception0x0B, 0,
                    IDT_TRAP_GATE);
  setGateDescriptor(0x0C, code_segment, &handle_exception0x0C, 0,
                    IDT_TRAP_GATE);
  setGateDescriptor(0x0D, code_segment, &handle_exception0x0D, 0,
                    IDT_TRAP_GATE);
  setGateDescriptor(0x0E, code_segment, &handle_exception0x0E, 0,
                    IDT_TRAP_GATE);
  setGateDescriptor(0x0F, code_segment, &handle_exception0x0F, 0,
                    IDT_TRAP_GATE);
  setGateDescriptor(0x10, code_segment, &handle_exception0x10, 0,
                    IDT_TRAP_GATE);
  setGateDescriptor(0x11, code_segment, &handle_exception0x11, 0,
                    IDT_TRAP_GATE);
  setGateDescriptor(0x12, code_segment, &handle_exception0x12, 0,
                    IDT_TRAP_GATE);
  setGateDescriptor(0x13, code_segment, &handle_exception0x13, 0,
                    IDT_TRAP_GATE);

  // Initialize PIC
  PIC_master_command_port.write(0x11);
  PIC_slave_command_port.write(0x11);
  // Remap the Interrupt Numbers
  PIC_master_data_port.write(hardware_interrupt_offset);
  PIC_slave_data_port.write(hardware_interrupt_offset + 8);
  // Setup PIC Master-Slave Connection
  PIC_master_data_port.write(0x04);
  PIC_slave_data_port.write(0x02);
  // Set PIC Mode
  PIC_master_data_port.write(0x01);
  PIC_slave_data_port.write(0x01);
  // Clear Data Registers
  PIC_master_data_port.write(0x00);
  PIC_slave_data_port.write(0x00);

  IDTPointer idt_pointer;
  idt_pointer.size = 256 * sizeof(GateDescriptor) - 1;
  idt_pointer.base = (uint32_t)idt_entries;
  asm volatile("lidt %0" : : "m"(idt_pointer));
}

uint16_t InterruptManager::hardwareInterruptOffset() {
  return hardware_interrupt_offset;
};

void InterruptManager::activate() {
  // sti (Set Interrupt Flag in EFLAGS), tells cpu to start accepting interrupts
  asm("sti");
}

void InterruptManager::deactivate() {
  // cli (Clear Interrupt Flag in EFLAGS), tells cpu to stop accepting
  // interrupts
  asm("cli");
}

// Generic ISR handler
extern "C" uint32_t handle_interrupt(uint8_t interrupt, uint32_t esp) {
  char foo[] = "INTERRUPT 0x00";
  char hex[] = "0123456789ABCDEF";
  foo[12] = hex[(interrupt >> 4) & 0xF];
  foo[13] = hex[interrupt & 0xF];

  uqaabOS::libc::Terminal terminal;
  terminal.print(foo);
  return esp;
}

} // namespace interrupts
} // namespace uqaabOS
