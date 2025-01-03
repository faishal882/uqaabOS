section .text
global load_gdt

load_gdt:
    ; Load the GDT descriptor address into %eax
    mov eax, [esp + 4]
    lgdt [eax]

    ; Load data segment selectors (0x10) into segment registers
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Perform far jump to reload code segment selector (0x08)
    jmp 0x08:.long_jump

.long_jump:
    ret

