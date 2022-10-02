/* Declare constants for the multiboot header. */
.set ALIGN,    1<<0             /* align loaded modules on page boundaries */
.set MEMINFO,  1<<1             /* provide memory map */
.set FLAGS,    ALIGN | MEMINFO  /* this is the Multiboot 'flag' field */
.set MAGIC,    0x1BADB002       /* 'magic number' lets bootloader find the header */
.set CHECKSUM, -(MAGIC + FLAGS) /* checksum of above, to prove we are multiboot */
 
/* 
Declare a multiboot header that marks the program as a kernel. These are magic
values that are documented in the multiboot standard. The bootloader will
search for this signature in the first 8 KiB of the kernel file, aligned at a
32-bit boundary. The signature is in its own section so the header can be
forced to be within the first 8 KiB of the kernel file.
*/
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM
.long 0
.long 0
.long 0
.long 0
.long 0
.long 0 # Set to 0 to enable framebuffer by default, 1 to disable
.long 640 # Number of horizontal pixels
.long 480 # Number of vertical pixels
.long 32 # Bit depth
.section .bss
.align 16
stack_bottom:
.skip 16384 # 16 KiB
stack_top:
 

.section .text
.global _start
.type _start, @function
_start:
	mov $stack_top, %esp
	pushl %ebx
	call arch_init
	pushl %ebx
	call kernel_main
	/* Issue description:
        Remove this if you wana add the switch to user mode function,
        because we receive #GF if don't do that, Thank you
        - Sergij Yevchuk*/
	#sti
1:
	jmp 1b
 
.size _start, . - _start
