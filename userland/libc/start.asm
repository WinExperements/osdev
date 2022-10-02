section .text
; We are using NASM syntax
extern main
global _start
extern exit
extern __env
_start:
    ; Clear stack, we currently doesn't have have argument passing
    ; Clear stack
    ; Debug
    pop eax
    ; Check if argument exists
    cmp eax,0
    je .exit
    push eax
    pop ebx
    mov [__env],ebx
    pop ebx
    push ebx
    push eax
    .run:
    call main
    .exit:
    push 0
    call exit
