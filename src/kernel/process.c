/* Preempective Process manager	*/
#include<process.h>
#include<mm/pmm.h>
#include<terminal.h>
#include <arch.h>
#include<x86/gdt.h>
#include<x86/isr.h>
#include<io.h>
#define THREAD_MAX 1024
struct process processList[THREAD_MAX];
typedef struct _state {
	bool fswitch; 	// first switchk
	uint32_t threadndx; 	// index of thread
} state;
state scs;
int clocks; // our timer clocks
void idle_task() {
	for (;;) {
	}
}
struct process *process_allocateProcess() {
	for (int i = 0; i < 1024; i++) {
		if (!processList[i].used) {
			processList[i].used = true;
			processList[i].pid = i;
			return &processList[i];
		}
	}
	return NULL;
}
/* Create process */
struct process *process_create(int entryPoint,bool isUser,char *name) {
	struct process *p = process_allocateProcess();
	if (p->used) {
		void *frame = arch_buildStack(entryPoint,isUser);
		p->esp = frame;
		p->name = name;
		p->kernelESP = (isUser ? ((int)pmml_alloc(true)) : 0);
		p->dir = (uint32_t)vmm_createDirectory();
		p->message_count = 0;
		processList[p->pid].esp = (void *)(int)frame;
	}
	return p;
}
void process_init() {
	for (int i = 0; i < 1024; i++) {
		processList[i].used = false;
	}
	process_create((int)idle_task,false,"idle");
	scs.fswitch = true;
}
void process_dump(struct process *pr) {
	printf("Process %d dump\n",pr->pid);
	printf("Address of stack: %x\n",pr->esp);
	printf("Name:%s\n",pr->name);
	printf("Kernel ESP: %d\n",pr->kernelESP);
	printf("Page directory address: %x\n",pr->dir);
	printf("Pages count: %d\n",pr->pages);
	printf("End of dump\n");
}
void process_yield() {
	PANIC("Not implemented");
}
void *process_schedule(void *stack) {
	// send end of interrupt to interrupt controller
	io_writePort(PIC_SLAVE_COMMAND , 0x20);
	io_writePort(PIC_MASTER_COMMAND , 0x20);
	// update clocks
	clocks++;
	// don't save context on first switch
	if (processList[scs.threadndx].state == 1) {
		struct process p = processList[scs.threadndx];
		pmml_free(p.esp);
		int *pr_dir = (int *)p.dir;
		for (int i = 0; i < p.pages; i++) {
			// free allocated pages
			int entry = pr_dir[i];
			if ((entry & 1) == 1) {
				int *table = (int *)(entry &~7);
				pmml_free(table);
		}
	}
		p.used = false;
		p.name = NULL;
		p.dir = 0;
		p.esp = 0;
		processList[scs.threadndx] = p;
	}
	if (!scs.fswitch) {
		processList[scs.threadndx].esp = stack;
	}
	if (!scs.fswitch) {
		for (scs.threadndx = (scs.threadndx+1) & 0xf; !processList[scs.threadndx].used && processList[scs.threadndx].state != 2; scs.threadndx = (scs.threadndx +1) & 0xf);
	}
	struct process *p = &processList[scs.threadndx];
	if (!p->used) {
		return stack;
	}
	if (p->wait_time != 0) {
		p->wait_time--;
		processList[p->pid] = *p;
		scs.fswitch = true;
		return stack;
	}
	tss_set_stack(0x10,p->kernelESP);
	// because we use paging switch it here
	vmm_switch((int *)p->dir);
	scs.fswitch = false;
	return p->esp;
}
int process_getCurrentPID() {
	return scs.threadndx;
}
void process_exit(int pid,int exitCode) {
	processList[pid].state = 1;
}
void process_kill(int pid) {
	process_exit(pid,-1);
}
void process_switchIdle() {
	//arch_executeTask(idleTask.esp);
}
struct process *process_getProcess(int pid) {
	if (pid >= 1024) {
		printf("No such process %d\n",pid);
		return NULL;
	}
	return &processList[pid];
}
void process_update(int pid,struct process *to) {
	if (pid >= 1024) {
		printf("No such process: %d\n",pid);
		return;
	}
	processList[pid] = *to;
}
void process_block(int pid) {
	processList[pid].state = 2; // blocked
}
void process_unblock(int pid) {
	processList[pid].state = 0;
}
void process_wait(int pid,int ms) {
	struct process p = processList[pid];
	p.wait_time = ms/10;
	processList[pid] = p;
}