#include "../../include/interrupts.h"
#include "../../include/libc/stdio.h"

namespace uqaabOS {
namespace interrupts {

InterruptManager *InterruptManager::ActiveInterrruptManager = 0;

InterruptHandler::InterruptHandler(InterruptManager *interrupt_manager,
                                   uint8_t interrupt_number) {
  this->interrupt_manager = interrupt_manager;
  this->interrupt_number = interrupt_number;
  interrupt_manager->handlers[interrupt_number] = this;
}

InterruptHandler::~InterruptHandler() {
  if (interrupt_manager->handlers[interrupt_number] != 0) {
    interrupt_manager->handlers[interrupt_number] = 0;
  }
}

uint32_t InterruptHandler::handle_interrupt(uint32_t esp) { return esp; }

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
                                   uqaabOS::include::GDT *gdt,
                                   multitasking::TaskManager *task_manager)
    : PIC_master_command_port(0x20), PIC_master_data_port(0x21),
      PIC_slave_command_port(0xA0), PIC_slave_data_port(0xA1) {

  this->task_manager = task_manager;

  this->hardware_interrupt_offset = hardware_interrupt_offset;
  // ISR code segment
  uint32_t code_segment = gdt->code_segment_selector();

  // Gate Type: 32-bit Interrupt Gate: 0xE
  const uint8_t IDT_INTERRUPT_GATE = 0xE;
  for (uint8_t i = 255; i > 0; --i) {
    setGateDescriptor(i, code_segment, &interrupt_ignore, 0,
                      IDT_INTERRUPT_GATE);
    handlers[i] = 0;
  }
  setGateDescriptor(0, code_segment, &interrupt_ignore, 0, IDT_INTERRUPT_GATE);
  handlers[0] = 0;

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

  if (ActiveInterrruptManager != 0) {
    ActiveInterrruptManager->deactivate();
  }

  ActiveInterrruptManager = this;
  // sti (Set Interrupt Flag in EFLAGS), tells cpu to start accepting interrupts
  asm("sti");
}

void InterruptManager::deactivate() {
  // cli (Clear Interrupt Flag in EFLAGS), tells cpu to stop accepting
  // interrupts
  if (ActiveInterrruptManager == this) {
    ActiveInterrruptManager = 0;
    asm("cli");
  }
}

// Generic ISR handler
extern "C" uint32_t handle_interrupt(uint8_t interrupt_number, uint32_t esp) {
  if (uqaabOS::interrupts::InterruptManager::ActiveInterrruptManager != 0) {
    return uqaabOS::interrupts::InterruptManager::ActiveInterrruptManager
        ->do_handle_interrupt(interrupt_number, esp);
  }

  return esp;
}

// Exception messages
const char *exception_messages[] = {"Divide By Zero Exception\n",
                                    "Debug Exception\n",
                                    "Non Maskable Interrupt Exception\n",
                                    "Breakpoint Exception\n",
                                    "Overflow Exception\n",
                                    "Bound Range Exceeded Exception\n",
                                    "Invalid Opcode Exception\n",
                                    "Device Not Available Exception\n",
                                    "Double Fault Exception\n",
                                    "Coprocessor Segment Overrun Exception\n",
                                    "Invalid TSS Exception\n",
                                    "Segment Not Present Exception\n",
                                    "Stack Fault Exception\n",
                                    "General Protection Fault Exception\n",
                                    "Page Fault Exception\n",
                                    "Unknown Interrupt Exception\n",
                                    "Coprocessor Fault Exception\n",
                                    "Alignment Check Exception\n",
                                    "Machine Check Exception\n",
                                    "Reserved Exception\n",
                                    "Reserved Exception\n"};

uint32_t InterruptManager::do_handle_interrupt(uint8_t interrupt_number,
                                               uint32_t esp) {
  if (handlers[interrupt_number] != 0) {
    esp = handlers[interrupt_number]->handle_interrupt(esp);
  } else if (interrupt_number != hardware_interrupt_offset) {
    if (interrupt_number <
        sizeof(exception_messages) / sizeof(exception_messages[0])) {
      libc::printf("EXCEPTION: %s", exception_messages[interrupt_number]);
    } else {
      libc::printf("UNHANDLED INTERRUPT 0x%02X\n", interrupt_number);
    }
  }

  if (interrupt_number == hardware_interrupt_offset) {
    esp = (uint32_t)(task_manager->schedule((multitasking::CPUState *)esp));
  }

  if (hardware_interrupt_offset <= interrupt_number &&
      interrupt_number < hardware_interrupt_offset + 16) {

    PIC_master_command_port.write(0x20);
    if (hardware_interrupt_offset + 8 <= interrupt_number) {
      PIC_slave_command_port.write(0x20);
    }
  }

  return esp;
}
} // namespace interrupts
} // namespace uqaabOS
