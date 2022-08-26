/* Preempective Process manager	*/
#include<process.h>
#include<mm/pmm.h>
#include<terminal.h>
#include <arch.h>
#include<x86/gdt.h>
#include<x86/isr.h>
#include<io.h>
#include<lib/clist.h>
#define THREAD_MAX 1024
#define PROCESS_RUNNING 4
int cPid;
int curTasks;
int clocks; // our timer clocks
bool fswitch;
struct process *runningTask;
clist_definition_t *task_list;
void idle_task() {
	for (;;) {
	}
}
struct process *process_findByStatus(int status);
struct process *process_findNextByStatus(int status,int addr);
struct process *process_findID(int id);
bool process_findDispatcher(clist_head_t *head,va_list args);
bool process_findIdDispatcher(clist_head_t *head,va_list args);
struct process *process_allocateProcess() {
	clist_head_t *h = clist_insert_entry_after(task_list,task_list->head);
	struct process *r = (struct process *)h->data;
	if (!r) {
		PANIC("Scheduler clist internal error.");
	}
	r->used = true;
	r->pid = cPid++;
	r->lAddr = (int)h;
	return r;
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
		p->esp = (void *)(int)frame;
		p->parent = process_getCurrentPID();
		p->state = PROCESS_RUNNING;
		struct process *pa = process_getProcess(p->parent);
		pa->parent = p->pid;
		curTasks++;
	}
	return p;
}
void process_init() {
	task_list = pmml_alloc(true);
	task_list->head = NULL;
	task_list->slot_size = sizeof(struct process);
	process_create((int)idle_task,false,"idle");
	//process_create((int)idle_task,false,"t");
	runningTask = NULL;
	fswitch = true;
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
	asm volatile("int $32");
}
void *process_schedule(void *stack) {
	// send end of interrupt to interrupt controller
	io_writePort(PIC_SLAVE_COMMAND , 0x20);
	io_writePort(PIC_MASTER_COMMAND , 0x20);
	struct process *next_task = NULL;
	// update clocks
	clocks++;
	// don't save context on first switch
	if (runningTask->state == 1) {
		if (runningTask->parent != 0) {
			struct process *parent = process_getProcess(runningTask->parent);
			if (parent->state == 2) {
				parent->state = PROCESS_RUNNING;
			}
		}
		curTasks--;
		runningTask->used = false;
		pmml_free(runningTask->esp);
		int *pr_dir = (int *)runningTask->dir;
		for (int i = 0; i < runningTask->pages; i++) {
			// free allocated pages
			int entry = pr_dir[i];
			if ((entry & 1) == 1) {
				int *table = (int *)(entry &~7);
				pmml_free(table);
		}
	}
		clist_delete_entry(task_list,(clist_head_t *)runningTask->lAddr);
		pmml_free(runningTask);
		runningTask = NULL; // let's scheduler find running task
	}
	if (!fswitch) {
		runningTask->esp = stack;
	}
	if (!fswitch) {
		if (!runningTask) {
			next_task = process_findByStatus(PROCESS_RUNNING);
		} else {
			next_task = process_findNextByStatus(PROCESS_RUNNING,runningTask->lAddr);
		}
	}
	if (next_task == NULL) {
		// first switch
		next_task = process_findByStatus(PROCESS_RUNNING);
	}
	if (next_task == NULL) {
		return stack;
	}
	if (next_task->wait_time != 0) {
		next_task->wait_time--;
		fswitch = true;
		return stack;
	}
	tss_set_stack(0x10,next_task->kernelESP);
	vmm_switch((int *)next_task->dir);
	fswitch = false;
	runningTask = next_task;
	return runningTask->esp;
}
int process_getCurrentPID() {
	return runningTask->pid;
}
void process_exit(int pid,int exitCode) {
	if (pid == 1) {
		PANIC("Init died.");
	}
	process_getProcess(pid)->state = 1;
}
void process_kill(int pid) {
	process_exit(pid,-1);
}
struct process *process_getProcess(int pid) {
	return process_findID(pid);
}
void process_update(int pid,struct process *to) {
}
void process_block(int pid) {
	process_getProcess(pid)->state = 2;
}
void process_unblock(int pid) {
	process_getProcess(pid)->state = 0;
}
void process_wait(int pid,int ms) {
	process_getProcess(pid)->wait_time = ms/10;
}
void process_waitPid(int pid) {
	if (runningTask->pid == 0 || pid < 0) return;
	if (!process_getProcess(pid)) return;
	process_getProcess(pid)->state = 2;  // waiting for a child
	process_yield();
}
struct process *process_findByStatus(int status) {
	clist_head_t *cur = clist_find(task_list,process_findDispatcher,status);
	if (cur) {
		return (struct process *)cur->data;
	}
	return NULL;
}
struct process *process_findNextByStatus(int status,int addr) {
	clist_head_t *cur = clist_find_next(task_list,(clist_head_t *)addr,process_findDispatcher,status);
	assert(cur != NULL);
	return (struct process *)cur->data;
}
struct process *process_findID(int id) {
	clist_head_t *c = clist_find(task_list,process_findIdDispatcher,id);
	if (c) {
		return (struct process *)c->data;
	}
	return NULL;
}
bool process_findDispatcher(clist_head_t *head,va_list args) {
	int status = va_arg(args,int);
	struct process *p = (struct process *)head->data;
	return p->state == status;
}
bool process_findIdDispatcher(clist_head_t *head,va_list args) {
	int id = va_arg(args,int);
	struct process *p = (struct process *)head->data;
	return p->pid == id;
}