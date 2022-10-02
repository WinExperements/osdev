#include<arch.h>
#include<serial.h>
#include<terminal.h>
#include<x86/gdt.h>
#include <interrupts.h>
#include <mm/pmm.h>
#include<io.h>
#include<process.h>
#define PUSH(tos,val) (*(-- tos) = val)
void gpf_exc(registers_t *);
void arch_init(struct multiboot_info *info) {
	terminal_initialize(info);
	init_serial();
	// now init gdt
	gdt_init();
	interrupts_init();
	interrupts_addHandler(13,gpf_exc);
	uint32_t esp;
    asm volatile("mov %%esp, %0" : "=r"(esp));
    tss_set_stack(0x10,esp);
#ifdef DEBUG
    write_serialString("X86 i386 code initialized(debug mode)\r\n");
#endif
}
void arch_reset() {
#ifdef DEBUG
    write_serialString("Reset!\r\n");
#endif
    asm volatile ("cli"); /* disable all interrupts */
 
   uint8_t good = 0x02;
    while (good & 0x02)
        good = io_readPort(0x64);
    io_writePort(0x64, 0xFE);
loop:
    asm volatile ("hlt"); 
	goto loop;
}
void panic(char *file,const char *funcName,const char *msg)
{
	// disable interrupts
	asm volatile("cli");
	terminal_setcolor(WHITE);
	terminal_setbackground(BLUE);
	printf("PANIC: %s:%s: %s\n",file,funcName,msg);
	for (;;) {}
}
void gpf_exc(registers_t *r) {
	printf("#GP error code: %d\n",r->error_code);
	PANIC("#GP");
}
void arch_disableIRQ() {
#ifdef DEBUG
    write_serialString("x86: disabling IRQ throuth PIC\r\n");
#endif
	io_writePort(0x21, 0xff);            // Disable all IRQs
   	io_writePort(0xa1, 0xff);             // Disable all IRQs
   	io_writePort(0x20, 0x11);
    io_writePort(0xa0, 0x11);
    io_writePort(0x21, 0x20);
    io_writePort(0xa1, 0x28);
    io_writePort(0x21, 0x04);
    io_writePort(0xa1, 0x02);
    io_writePort(0x21, 0x01);
   	io_writePort(0xa1, 0x01);
}
void arch_enableIRQ() {
#ifdef DEBUG
    write_serialString("x86: enabling IRQ throuth PIC\r\n");
#endif
	io_writePort(0x21, 0x00);            // Enable all IRQs
   	io_writePort(0xa1, 0x00);             // Enable all IRQs
}
void *arch_buildStack(int entryPoint,bool isUser) {
#ifdef DEBUG
    write_serialString("x86: building process stack\r\n");
#endif
	registers_t *frame = (pmml_alloc(true));
	frame->eflags = 0x202;
	frame->cs    = (isUser ? 0x1b : 0x8);
 	frame->eip   = entryPoint;
 	frame->ebp   = 0;
  	frame->edi   = 0;
  	frame->esi   = 0;
  	frame->edx   = 0;
  	frame->ecx   = 0;
  	frame->ebx   = 0;
  	frame->eax   = 0;
  	frame->ds    = (isUser ? 0x23 : 0x10);
    frame->ss = frame->ds;
	frame->useresp = (isUser ? ((int)pmml_alloc(true)+4096) : 0);
	return (void *)(int)frame;
}
void arch_disableInterrupts() {
	asm volatile("cli");
}
void arch_enableInterrupts() {
	asm volatile("sti");
}
void arch_poweroff() {
	io_writePortW(0xB004, 0x2000);
    io_writePortW(0x604, 0x2000);
    io_writePortW(0x4004, 0x3400);
    arch_disableInterrupts();
    printf("HALT\n");
    while (1){}
}
void arch_copy_process_args(struct process *p,int argc,char **argv,char **envp) {
#ifdef DEBUG
    write_serialString("x86: copying user arguments into stack\r\n");
#endif
	// First we need to get the user stack, but first check if task are in user mode
	registers_t *fr = (registers_t *)p->esp;
	int *stack = (int *)fr->useresp;
    if (stack == NULL) {
        // Kernel mode thread
#ifdef DEBUG
        write_serialString("x86: warrning kernel thread will not use arguments!\r\n");
#endif
        return;
    }
	PUSH(stack,(int)argv);
	PUSH(stack,argc);
    PUSH(stack,(int)envp);
	fr->useresp = (int)stack;
#ifdef DEBUG
    	write_serialString("x86: arguments pushed\r\n");
#endif
	// Here we done!
}
void arch_destroyStack(void *stack) {
#ifdef DEBUG
    write_serialString("Destroying stack\r\n");
#endif
	registers_t *fr = (registers_t *)stack;
	if (fr->useresp != 0) {
		pmml_free((void *)fr->useresp-4);
        pmml_free((void *)fr->useresp-12);
		pmml_free((void *)fr->useresp);
	}
}
void arch_switchContext(void *stack) {
    asm volatile("movl %%eax,%%esp" : : "a" (stack));
    asm volatile("jmp irq_handler_exit");
}
