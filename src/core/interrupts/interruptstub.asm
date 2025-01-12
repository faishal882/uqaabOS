; Define IRQ base
IRQ_BASE equ 0x20

; External reference to C handler function
extern handle_interrupt

section .text

; Exception handler macro
%macro HandleException 1
global handle_exception%1
handle_exception%1:
    mov dword [interrupt_number], %1 
    jmp int_bottom
%endmacro

; IRQ handler macro
%macro HandleInterruptRequest 1
global IRQ%1
IRQ%1:
    mov dword [interrupt_number], (%1 + IRQ_BASE) ;push byte (%1 + IRQ_BASE)
    jmp int_bottom
%endmacro

; Generate exception handlers
HandleException 0x00
HandleException 0x01
HandleException 0x02
HandleException 0x03
HandleException 0x04
HandleException 0x05
HandleException 0x06
HandleException 0x07
HandleException 0x08
HandleException 0x09
HandleException 0x0A
HandleException 0x0B
HandleException 0x0C
HandleException 0x0D
HandleException 0x0E
HandleException 0x0F
HandleException 0x10
HandleException 0x11
HandleException 0x12
HandleException 0x13

; Generate IRQ handlers
HandleInterruptRequest 0x00
HandleInterruptRequest 0x01
HandleInterruptRequest 0x02
HandleInterruptRequest 0x03
HandleInterruptRequest 0x04
HandleInterruptRequest 0x05
HandleInterruptRequest 0x06
HandleInterruptRequest 0x07
HandleInterruptRequest 0x08
HandleInterruptRequest 0x09
HandleInterruptRequest 0x0A
HandleInterruptRequest 0x0B
HandleInterruptRequest 0x0C
HandleInterruptRequest 0x0D
HandleInterruptRequest 0x0E
HandleInterruptRequest 0x0F
HandleInterruptRequest 0x31

; Common handler code
int_bottom:
    pusha
    push ds
    push es
    push fs
    push gs
    
    push esp
    push dword [interrupt_number]
    call handle_interrupt   ; Call the C-style handler function
    add esp, 8
    mov esp, eax
    
    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 4      ; Clean up the interrupt number we pushed
    iret

; Default interrupt handler
global interrupt_ignore
interrupt_ignore:
    iret

section .data
    interrupt_number: dd 0
