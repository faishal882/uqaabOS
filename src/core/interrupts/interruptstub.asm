; Define the base address for IRQ handlers - IRQs are mapped starting at interrupt 0x20
IRQ_BASE equ 0x20

; Declare external C function that will handle the interrupts
extern handle_interrupt

; Start of code section
section .text

; Macro for handling exceptions that push error codes on the stack automatically
%macro HandleExceptionWithError 1 
global handle_exception%1                   ; Make the handler visible to linker
handle_exception%1:
    mov dword [interrupt_number], %1        ; Store the exception number in global variable
    jmp int_bottom                         ; Jump to common handler code
%endmacro

; Macro for handling exceptions that don't push error codes
%macro HandleException 1 
global handle_exception%1                   ; Make the handler visible to linker
handle_exception%1:
    mov dword [interrupt_number], %1        ; Store the exception number in global variable
    push 0                                 ; Push a dummy error code (0) for consistency
    jmp int_bottom                         ; Jump to common handler code
%endmacro

; Macro for handling IRQ (hardware interrupts)
%macro HandleInterruptRequest 1 
global IRQ%1                               ; Make the IRQ handler visible to linker
IRQ%1:
    mov dword [interrupt_number], (%1 + IRQ_BASE)  ; Store IRQ number offset by base
    push 0                                 ; Push a dummy error code (0) for consistency
    jmp int_bottom                         ; Jump to common handler code
%endmacro

; Generate handlers for all standard x86 exceptions
HandleException 0x00     ; Division by Zero Exception
HandleException 0x01     ; Debug Exception
HandleException 0x02     ; Non Maskable Interrupt Exception
HandleException 0x03     ; Breakpoint Exception
HandleException 0x04     ; Into Detected Overflow Exception
HandleException 0x05     ; Out of Bounds Exception
HandleException 0x06     ; Invalid Opcode Exception
HandleException 0x07     ; No Coprocessor Exception

; Generate handlers for exceptions that push error codes
HandleExceptionWithError 0x08     ; Double Fault Exception
HandleException 0x09             ; Coprocessor Segment Overrun Exception
HandleExceptionWithError 0x0A     ; Bad TSS Exception
HandleExceptionWithError 0x0B     ; Segment Not Present Exception
HandleExceptionWithError 0x0C     ; Stack Fault Exception
HandleExceptionWithError 0x0D     ; General Protection Fault Exception
HandleExceptionWithError 0x0E     ; Page Fault Exception
HandleException 0x0F             ; Unknown Interrupt Exception
HandleException 0x10             ; Coprocessor Error Exception
HandleExceptionWithError 0x11     ; Alignment Check Exception
HandleException 0x12             ; Machine Check Exception
HandleException 0x13             ; SIMD Floating-Point Exception

; Generate handlers for all IRQs
HandleInterruptRequest 0x00      ; IRQ0  - Programmable Interrupt Timer
HandleInterruptRequest 0x01      ; IRQ1  - Keyboard
HandleInterruptRequest 0x02      ; IRQ2  - Cascade for IRQ8-IRQ15
HandleInterruptRequest 0x03      ; IRQ3  - COM2
HandleInterruptRequest 0x04      ; IRQ4  - COM1
HandleInterruptRequest 0x05      ; IRQ5  - LPT2
HandleInterruptRequest 0x06      ; IRQ6  - Floppy Disk
HandleInterruptRequest 0x07      ; IRQ7  - LPT1
HandleInterruptRequest 0x08      ; IRQ8  - CMOS Real Time Clock
HandleInterruptRequest 0x09      ; IRQ9  - Free
HandleInterruptRequest 0x0A      ; IRQ10 - Free
HandleInterruptRequest 0x0B      ; IRQ11 - Free
HandleInterruptRequest 0x0C      ; IRQ12 - PS2 Mouse
HandleInterruptRequest 0x0D      ; IRQ13 - FPU
HandleInterruptRequest 0x0E      ; IRQ14 - Primary ATA
HandleInterruptRequest 0x0F      ; IRQ15 - Secondary ATA
HandleInterruptRequest 0x31      ; Custom IRQ handler
HandleInterruptRequest 0x80      ; IRQ80 - System Call


; Common interrupt handling code
int_bottom:
    ; Save registers
    push ebp
    push edi
    push esi
    push edx
    push ecx
    push ebx
    push eax

    ; Call the C handler function
    push esp                            ; Push stack pointer as parameter
    push dword [interrupt_number]        ; Push interrupt number as parameter
    call handle_interrupt               ; Call C handler function
    mov esp, eax                        ; Update stack pointer from return value

    ; Restore registers
    pop eax
    pop ebx
    pop ecx
    pop edx
    pop esi
    pop edi
    pop ebp

    ; Clean up the stack (error code or dummy value)
    add esp, 4

    ; Return from interrupt
    iret

; Default handler for unhandled interrupts
global interrupt_ignore
interrupt_ignore:
    iret                                ; Immediately return from interrupt

; Data section for global variables
section .data
    interrupt_number: dd 0              ; Storage for current interrupt number