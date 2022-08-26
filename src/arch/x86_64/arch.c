/**  X86-64 architecture dependes **/
// Were are use currently the terminal implementation of X86 terminal
#include<terminal.h>
#include<arch.h>
#include<typedefs.h>
void arch_init(multiboot_info_t *info) {
    terminal_initialize(info);
    printf("HelinOS X86-64 support test, now IDLE loop\n");
    while(1) {}
}
// create dummy functions for property compilling
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
void arch_disableIRQ() {}
void arch_enableIRQ() {}
void *arch_buildStack(int entryPoint,bool isUser) {
    return NULL;
}
void arch_disableInterrupts() {
	asm volatile("cli");
}
void arch_enableInterrupts() {
	asm volatile("sti");
}
void arch_reset() {
    
}