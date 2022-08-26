.global _start
_start:
    ldr sp,=STACK_TOP
    mov r1,#0
    push {r1}
    bl arch_setup
    ; If we are here, architecture init complete, now jump to our kernel entry point
    ; Push into stack address 0x0 of currently unused paramter, aka multiboot in X86
    mov r1,#0
    push r1
    bl kernel_main
1:
    b 1b
.size _start, . - _start