#include<arch.h>
#include<serial.h>
#include<terminal.h>
#include<x86/gdt.h>
#include <interrupts.h>
#include <mm/pmm.h>
#include<io.h>
#include<process.h>
#include<elf.h>
#define PUSH(tos,val) (*(-- tos) = val)
void gpf_exc(registers_t *);
void x86_userSwitch(int entryPoint,int user_esp);
void x86_switchStacks(registers_t *regs);
typedef struct _x86_arch_task {
    int iframe;
    int ustack;
} x86_arch_task;
struct process *__active;
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
void *arch_buildStack(int entryPoint,bool isUser,int argc,char **argv) {
#ifdef DEBUG
    write_serialString("x86: building process stack\r\n");
#endif
	int *s = pmml_alloc(true);
    if (isUser) { // remember: kernel tasks/threads never use arguments!
        /*
         * If task is user then allocate user esp and set
         * entry point to x86_userSwitch
         */
        int *user_esp = pmml_alloc(true);
        PUSH(user_esp,(int)argv);
        PUSH(user_esp,argc);
        PUSH(s,(int)user_esp);
        PUSH(s,entryPoint);
        PUSH(s,0);
        PUSH(s,(int)x86_userSwitch);
        /*PUSH(s,(int)argv);
        PUSH(s,argc);*/
    } else {
        PUSH(s,0);
        PUSH(s,entryPoint);
    }
    //PUSH(s,entryPoint);
    PUSH(s,0);
    PUSH(s,0);
    PUSH(s,0);
    PUSH(s,0);
	return (void *)(int)s;
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
	// Here we done!
}
void arch_destroyStack(void *stack) {
    // Destroy our arch_task
    x86_arch_task *task = (x86_arch_task *)stack;
    // Free our IFRAME and user stack
    pmml_free((void *)task->iframe);
    pmml_free((void *)task->ustack);
}
void x86_userSwitch(int entryPoint,int stack) {
    /* 
     * Description of this code:
     * We are allocate and fill our interrupt frame, 
     * after that we just restore it using irq_handler_exit
     * after it we in our userspace entry point and
     * user mode selectors, so after the timer interrupt
     * invoked and scheduler called arch_switchContext it will do nothing,
     * so our IRQ handler restored our user space selectors
     * and returns to entry point.
     * 
     * USER MODE FINALLY WORKS!
     */
    registers_t *regs = pmml_alloc(true);
    regs->eax = regs->ecx = regs->edx = regs->ebx = regs->ebp = regs->esi = regs->edi = 0;
    regs->ds = 0x23;
    regs->cs = 0x1b;
    regs->eip = entryPoint;
    regs->ss = regs->ds;
    regs->eflags = 0x200;
    regs->useresp = stack;
    // Now restore it using our assembly code
    // before the actual switch, create and fill our arch_task
    if (__active != NULL) {
        x86_arch_task *a = pmml_alloc(true);
        a->iframe = (int)regs;
        a->ustack = stack;
        __active->arch_task = (void *)a;
    }
    x86_switchStacks(regs);
}
void arch_set_active_thread(struct process *active) {
    __active = active;
}
bool arch_relocSymbols(module_t *mod,void *ehdr) {
	Elf32_Ehdr *e = (Elf32_Ehdr *)ehdr;
	Elf32_Shdr *s;
	Elf32_Word entsize;
	unsigned i;
	// Find symbol table
	for (i = 0,s = (Elf32_Shdr *)((char *) e + e->e_shoff); i < e->e_shnum;
		i++,s = (Elf32_Shdr *)((char *) s + e->e_shentsize)) {
		if (s->sh_type == SHT_SYMTAB) {
			break;
		}
	}
	if (i == e->e_shnum) {
		printf("relloc: no symbol table to relocate!\n");
		return false;
	}
	entsize = s->sh_entsize;
	for (i = 0,s = (Elf32_Shdr *)((char *)e + e->e_shoff); i < e->e_shnum;
		i++,s = (Elf32_Shdr *)((char *)s + e->e_shentsize)) {
			if (s->sh_type == SHT_REL) {
				printf("Relocation segment\n");
				module_segment_t *seg;
				for (seg = mod->seg; seg; seg = seg->next) {
					if (seg->section == s->sh_info) break;
				}
				if (seg) {
					Elf32_Rel *rel,*max;
					for (rel = (Elf32_Rel *)((char *) e + s->sh_offset),
						max = rel + s->sh_size / s->sh_entsize;
						rel < max;
						rel++) {
						Elf32_Word *addr;
						Elf32_Sym *sym;
						if (seg->size < rel->r_offset) {
							printf("Relloc offset is out of segment\n");
							return false;
						}
						addr = (Elf32_Word *)((char *)seg->addr + rel->r_offset);
						sym = (Elf32_Sym *)((char *)mod->symtab + entsize * ELF32_R_SYM(rel->r_info));
						switch (ELF32_R_TYPE(rel->r_info)) {
							case R_386_32:
								*addr += sym->st_value;
								break;
							case R_386_PC32:
								*addr += (sym->st_value - (Elf32_Word)seg->addr - rel->r_offset);
								break;
							default:
							printf("Unknown relocation type!\n");
							break;
						}
					}
				}
			}
	}
	return true;
}
