#ifndef __MULTITASKING_
#define __MULTITASKING_

#include <stdint.h>

#include "../gdt.h"

namespace uqaabOS
{
    namespace multitasking
    {
        
        
        struct CPUState{

         uint32_t eax;
         uint32_t ebx;
         uint32_t ecx;
         uint32_t edx;

         uint32_t esi;
         uint32_t edi;
         uint32_t ebp;

         uint32_t error;

         uint32_t eip;
         uint32_t cs;
         uint32_t eflags;
         uint32_t esp;
         uint32_t ss;

        } __attribute__((packed));


        class Task{
            friend class TaskManager;
            private:
            uint8_t stack[4096];
            CPUState* cpu_state;

            public:
            Task(include::GDT* gdt , void entry_point());
            ~Task();
        };

        class TaskManager{

            private:

            Task* tasks[256];
            int num_tasks;
            int current_task;

            public:

            TaskManager();
            ~TaskManager();
            bool add_task(Task* task);
            CPUState* schedule(CPUState* cpu_state);

        };
    } // namespace multitasking
    
    
} // namespace uqaabOS

#endif