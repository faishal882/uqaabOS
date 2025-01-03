section .text
    global load_gdt

load_gdt:
    ; Load the GDT pointer (GDT descriptor) from the stack into EAX
    mov eax, [esp + 4]   
    lgdt [eax]           ; Load GDT from the address in EAX
    
    ; Set up the segment registers to point to the new GDT segments
    mov ax, 0x10         ; Code/Data segment selector, typically 0x10 for code/data segments
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Perform a far jump to load the new segments
    jmp 0x8:.long_jump   ; Jump to the new code segment (0x8 is a typical code segment selector)

.long_jump:
    ret                  ; Return from the function (with new segments active)

