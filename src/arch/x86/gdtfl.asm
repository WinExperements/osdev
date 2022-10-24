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
[GLOBAL arch_jumpToUser]
arch_jumpToUser:
    mov eax,[esp+4]
    mov ebx,[esp+8]
    mov cx, 0x23
    mov ds, cx
    mov es, cx
    mov fs, cx
    mov gs, cx

    push 0x23
    push ebx
    pushf
    ; code segment selector, rpl = 3
    push 0x1b
    push eax
    sti
    iretd
user_start:
    ret
	
[global x86_user_exit]
x86_user_exit:
ret

[extern x86_user_exit]
arch_switchToUser:
    mov cx, 0x23
    mov ds, cx
    mov es, cx
    mov fs, cx
    mov gs, cx
    mov eax,esp
    push 0x23
    push eax
    push 0x200
    push 0x1b
    push x86_user_exit
    iretd
[global arch_switchContext]
[extern runningTask]
arch_switchContext:
    push ebx
    push esi
    push edi
    push ebp
    mov edi,[runningTask]
    mov [edi+0],esp
    mov esi,[esp+((4+1)*4)]
    mov [runningTask],esi
    mov esp,[esi+0]
    pop ebp
    pop edi
    pop esi
    pop ebx
    ret

    
[global x86_switchStacks]
[extern irq_handler_exit]
; arch_switchStacks description
; We just change the esp to our first paramter
; and restors it
x86_switchStacks:
    mov ebp,[esp+4] ; Get our regs argument
    mov esp,ebp
    jmp irq_handler_exit
