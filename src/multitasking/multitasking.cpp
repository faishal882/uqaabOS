#include "../include/multitasking/multitasking.h"
// #include "../include/gdt.h"

#include "../include/gdt.h"

namespace uqaabOS {
namespace multitasking {

Task::Task(uqaabOS::include::GDT *gdt, void (*entry_point)()) {
  // Place CPUState at the top of the stack (highest address)
  cpu_state = (CPUState *)(stack + 4096 - sizeof(CPUState));
  // static_assert(sizeof(CPUState) <= 4096, "CPUState too large for stack");

  // Initialize registers
  cpu_state->eax = 0;
  cpu_state->ebx = 0;
  cpu_state->ecx = 0;
  cpu_state->edx = 0;
  cpu_state->esi = 0;
  cpu_state->edi = 0;
  cpu_state->ebp = 0;

  // Set esp to the TOP of the stack (stack + 4096)
  cpu_state->esp = (uint32_t)(stack + 4096);

  // Set execution context
  cpu_state->eip = (uint32_t)entry_point;
  cpu_state->ss = gdt->data_segment_selector();
  cpu_state->cs = gdt->code_segment_selector();
  cpu_state->eflags = 0x202; // Enable interrupts
}

Task::~Task() {}

TaskManager::TaskManager() {
  num_tasks = 0;
  current_task = -1;
}

TaskManager::~TaskManager() {}

bool TaskManager::add_task(Task *task) {

  if (num_tasks >= 256)
    return false;

  tasks[num_tasks++] = task;

  return true;
}

CPUState *TaskManager::schedule(CPUState *cpu_state) {
  if (num_tasks <= 0) {
    return cpu_state;
  }

  // Save current task's state
  if (current_task >= 0) {
    tasks[current_task]->cpu_state = cpu_state;
  }

  // Select next task
  if (++current_task >= num_tasks) {
    current_task = 0;
  }

  // Return next task's state
  return tasks[current_task]->cpu_state;
}
} // namespace multitasking

} // namespace uqaabOS