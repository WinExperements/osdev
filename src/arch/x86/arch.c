#include<arch.h>
#include<serial.h>
#include<terminal.h>
#include<x86/gdt.h>
#include <interrupts.h>
#include <mm/pmm.h>
#include<io.h>
#include<process.h>
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
}
void arch_reset() {
	__asm("jmp 0x0");
}
void panic(char *file,const char *funcName,const char *msg)
{
	// disable interrupts
	asm volatile("cli");
	terminal_setcolor(WHITE);
	terminal_setbackground(BLUE);
	printf("A problem detected, and system has halted.\n");
	printf("PANIC: %s:%s: %s\n",file,funcName,msg);
	for (;;) {}
}
void gpf_exc(registers_t *r) {
	printf("#GP error code: %d\n",r->error_code);
	PANIC("#GP");
}
void arch_executeTask(int esp) {
	asm volatile("mov %0,%%esp" : : "r" (esp));
	asm volatile("popal");
	asm volatile("sti");
	asm volatile("iretl");
}
void arch_test(int oldesp,int newesp) {
	asm volatile("movl %%esp,%0" : "=r" (oldesp));
	asm volatile("movl %0,%%esp" : : "r" (newesp));
}
void arch_disableIRQ() {
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
	io_writePort(0x21, 0x00);            // Enable all IRQs
   	io_writePort(0xa1, 0x00);             // Enable all IRQs
}
void *arch_buildStack(int entryPoint,bool isUser) {
	stackFrame *frame = (pmml_alloc(true));
	frame->flags = 0x202;
	frame->cs    = (isUser ? 0x1b : 0x8);
 	frame->eip   = entryPoint;
 	frame->ebp   = 0;
  	frame->esp   = 0;
  	frame->edi   = 0;
  	frame->esi   = 0;
  	frame->edx   = 0;
  	frame->ecx   = 0;
  	frame->ebx   = 0;
  	frame->eax   = 0;
  	frame->ds    = (isUser ? 0x23 : 0x10);
	frame->es = frame->ds;
	frame->fs = frame->ds;
	frame->gs = frame->ds;
	frame->ss = frame->ds;
	frame->usersp = (isUser ? ((int)pmml_alloc(true)+1024) : 0);
	return (void *)(int)frame;
}
