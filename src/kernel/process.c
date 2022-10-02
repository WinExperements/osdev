/* Preempective Process manager	*/
#include<process.h>
#include<mm/pmm.h>
#include<terminal.h>
#include <arch.h>
#include<x86/gdt.h>
#include<x86/isr.h>
#include<io.h>
#include<serial.h>
#include<mm.h>
#include<mstring.h>
#include<interrupts.h>
#include<dev.h>
#include<kshell.h>
#define PROCESS_RUNNING 4
int cPid;
int curTasks;
int clocks; // our timer clocks
bool fswitch;
struct process *runningTask;
clist_definition_t *task_list;
struct process *idle;
bool disableScheduler;
void __process_destroy(struct process *task);
int _priv_syscall(int num,int p1,int p2,int p3,int p4,int p5) {
    // use asm macro for it
    int ret = 0;
    asm volatile("int %1\n" 
                : "=a" (ret) 
                : "i" (0x80),
                "a" (num),
                "d" (p1),
                "c" (p2),
                "b" (p3),
                "D" (p4),
                "S" (p5)
                : "cc", "memory");
    return ret;
}
void idle_task() {
    // test execute kshell using it
    for (;;) {}
}
struct process *process_findByStatus(int status);
struct process *process_findNextByStatus(int status,int addr);
struct process *process_findID(int id);
bool process_findDispatcher(clist_head_t *head,va_list args);
bool process_findIdDispatcher(clist_head_t *head,va_list args);
struct process *process_allocateProcess() {
#ifdef DEBUG
    write_serialString("allocating process!\r\n");
#endif
    if (task_list == NULL || (runningTask != NULL && task_list->head == NULL)) {
        PANIC("Task manager list are NULL!");
    }
	clist_head_t *h = clist_insert_entry_after(task_list,task_list->head);
#ifdef DEBUG
    write_serialString("proc: clist entry inserted\r\n");
#endif
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
    // Before creating any process, disable scheduler!
    disableScheduler = true;
#ifdef DEBUG
    write_serialString("Scheduler: creating process\r\n");
#endif
	struct process *p = process_allocateProcess();
	if (p->used) {
		void *frame = arch_buildStack(entryPoint,isUser);
		p->esp = frame;
		p->name = name;
		p->kernelESP = (isUser ? ((int)pmml_alloc(true)+4096) : 0);
		p->dir = ((isUser ? 0 : (int)vmm_getCurrentDirectory()));
		p->message_count = 0;
		p->esp = (void *)(int)frame;
		p->parent = process_getCurrentPID();
		p->state = PROCESS_RUNNING;
		struct process *pa = process_getProcess(p->parent);
        if (pa == NULL) {
            arch_destroyStack(frame);
            pmml_free(p);
            printf("W: No such parent %d!\n",p->parent);
            disableScheduler = false;
            return NULL;
        }
		p->workDir = vfs_getRoot();
        p->user = isUser;
		curTasks++;
        disableScheduler = false;
	}
	return p;
}
void process_init() {
	task_list = pmml_alloc(true);
    if (task_list == NULL) {
        PANIC("Task manager: allocated list are null");
    }
	task_list->head = NULL;
	task_list->slot_size = sizeof(struct process);
	// register our interrupt handler here
	idle = process_create((int)idle_task,false,"idle");
    idle->state = 0;
	//fswitch = true;
    runningTask = NULL;
    interrupts_addHandler(32,process_schedule);
#ifdef DEBUG
    write_serialString("proc: waitpid,kill,exit: not implemented!\r\n");
#endif
}
void process_dump(struct process *pr) {
    if (pr == NULL) {
        printf("process_dump: em, process are null\n");
        return;
    }
	printf("Process %d dump\n",pr->pid);
	printf("Address of stack: %x\n",pr->esp);
	printf("Name:%s\n",pr->name);
	printf("Kernel ESP: %d\n",pr->kernelESP);
	printf("Page directory address: %x\n",pr->dir);
	printf("Pages count: %d\n",pr->pages);
	printf("State: %d\n",pr->state);
	printf("Parent: %d\n",pr->parent);
    printf("Entry point: %x\n",(((registers_t *)pr->esp)->eip));
	printf("End of dump\n");
}
void process_yield() {
	asm volatile("int $32");
}
void process_schedule(registers_t *stack) {
	// send end of interrupt to interrupt controller
    // Simple, just find a task then switch to it.
    // Find any task that need to be killed
    if (disableScheduler) return;
    struct process *kill = process_findByStatus(PROCESS_KILLING);
    if (kill != NULL) {
        __process_destroy(kill);
        if (runningTask == kill) {
            runningTask = NULL;
        }
    }
    if (runningTask != NULL) {
        // Simply select new task
        runningTask->esp = (void *)stack;
        /*write_serialString("Save for ");
        write_serialString(runningTask->name);
        write_serialString("\r\n");*/
        runningTask = process_findNextByStatus(PROCESS_RUNNING,runningTask->lAddr);
        runningTask->quota = 0;
    } else {
        // first switch
        runningTask = process_findByStatus(PROCESS_RUNNING);
    }
    if (runningTask == NULL) {
        // okay switch to idle
        runningTask = idle;
    }
    // change the TSS and VMM
    tss_set_stack(0x10,runningTask->kernelESP);
    vmm_switch((int *)runningTask->dir);
	arch_switchContext(runningTask->esp);
}
int process_getCurrentPID() {
	if (runningTask == NULL) {
		return 0;
	} else {
		return runningTask->pid;
	}
}
void process_exit(int pid,int exitCode) {
    arch_disableInterrupts();
	fswitch = true;
    if (pid == 1) {
        // only test!
        // re-create init
        PANIC("Init died, or exited");
    }
	process_getProcess(pid)->state = 1;
    arch_enableInterrupts();
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
    if (pid == runningTask->pid) {
        runningTask = NULL;
    }
	process_getProcess(pid)->state = 2;
}
void process_unblock(int pid) {
	process_getProcess(pid)->state = PROCESS_RUNNING;
}
void process_wait(int pid,int ms) {
	process_getProcess(pid)->wait_time = ms/10;
}
void process_waitPid(int pid) {
    arch_disableIRQ();
	arch_disableInterrupts();
	if (pid < 0) return;
	if (!process_getProcess(pid)) {
		 printf("process_wait: no such process %d, system halted\n",pid); 
		 return;
	}
	struct process *f = process_getProcess(pid);
    f->state = PROCESS_WAITPID;
    f->quota = PROCESS_QUOTA;
	// update pointer
	arch_enableIRQ();
	arch_enableInterrupts();
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
	if (cur == NULL) {
        return NULL;
    }
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
void __process_destroy(struct process *task) {
	if (true) { // idle task hasn't waiting for any pid
		struct process *parent = process_findID(task->parent);
        	if (parent == NULL) {
            		printf("W: parent %d of %d null, reseting running task!\n",task->parent,task->pid);
            		runningTask = NULL;
         	}
		process_unblock(parent->pid);
		curTasks--;
		arch_destroyStack(task->esp);
		pmml_free(task->esp);
		if (task->user) {
            		pmml_free((void *)task->dir);
        }
		if (task->kernelESP != 0) {
			pmml_free((void *)task->kernelESP);
		}
		if (task->page_start != 0) {
			for (int i = 0; i < task->pages; i++) {
                int addr = vmm_translate((int *)task->dir,task->page_start+(i*4096));
                pmml_free((void *)addr);
            }
		}
		clist_delete_entry(task_list,(clist_head_t *)task->lAddr);
		pmml_free(task);
	}
}
int process_getProcesses() {
    return curTasks;
}
void process_dumpAll() {
    struct process *f = process_findByStatus(PROCESS_RUNNING);
    struct process *start = f;
    while(f != NULL) {
        printf("%d      %s      %d\n",f->pid,f->name,f->parent);
        if (f == start) break;
        f = process_findNextByStatus(PROCESS_RUNNING,f->lAddr);
    }
}
void process_disableScheduler() {
    disableScheduler = true;
}
void process_enableScheduler() {
    disableScheduler = false;
}
int process_getNextPID() {
    return cPid;
}
