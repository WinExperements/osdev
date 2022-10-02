[GLOBAL gdt_flush]
gdt_flush:              ; Define the Functions
    mov eax , [esp+4]   ; Get the First Parameter which is the GDT pointer
    lgdt[eax]           ; Load the GDT using our pointer

    mov ax, 0x10        ; 0x10 is the offset in the GDT to our data segment
    mov ds, ax          ; Load all data segment selectors
    mov es, ax
    mov fs, ax
    mov gs, ax
    jmp 0x08:gdt_flush_end
gdt_flush_end:
	ret
[GLOBAL tss_flush]
tss_flush:
    mov ax,0x2B
    ltr ax
    ret
[GLOBAL arch_switchToUser]
arch_switchToUser:
  mov ax, 0x23
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push 0x23
    push esp
    pushfd
    ; code segment selector, rpl = 3
    push 0x1b
    lea eax, [user_start]
    push eax
    sti
    iretd
user_start:
    add esp, 4
    ret
	