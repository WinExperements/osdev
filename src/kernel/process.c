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
void idle_task() {
    // test execute kshell using it
    arch_enableInterrupts();
    for (;;) {}
}
struct process *process_findByStatus(int status);
struct process *process_findNextByStatus(int status,int addr);
struct process *process_findID(int id);
bool process_findDispatcher(clist_head_t *head,va_list args);
bool process_findIdDispatcher(clist_head_t *head,va_list args);
file_descriptor_t *process_findFDDispatcher(clist_head_t *head,va_list args);
void process_closeDispatcher(clist_head_t *head,va_list args);
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
struct process *process_create(int entryPoint,bool isUser,char *name,int argc,char **argv) {
    // Before creating any process, disable scheduler!
    disableScheduler = true;
#ifdef DEBUG
    write_serialString("Scheduler: creating process\r\n");
#endif
	struct process *p = process_allocateProcess();
	if (p->used) {
		void *frame = arch_buildStack(entryPoint,isUser,argc,argv);
		p->esp = frame;
        	arch_set_active_thread(p);
		p->name = name;
		p->kernelESP = (isUser ? ((int)pmml_alloc(true)+4096) : 0);
		p->dir = ((isUser ? 0 : (int)vmm_getCurrentDirectory()));
		p->message_count = 0;
		p->esp = (void *)(int)frame;
		p->parent = process_getCurrentPID();
		p->state = PROCESS_RUNNING;
		// UPDATE: Allocate FD list for single process
		p->fds = pmml_alloc(true);
		p->fds->head = NULL;
		p->fds->slot_size = sizeof(file_descriptor_t); 
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
        p->uid = pa->uid;
        p->guid = pa->guid;
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
	idle = process_create((int)idle_task,false,"idle",0,NULL);
    idle->state = 0;
	//fswitch = true;
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
    registers_t *regs = (registers_t *)pr->esp;
    printf("Entry point: %x\n",regs->eip);
    printf("User mode:  %x\n",regs->cs);
	printf("End of dump\n");
}
void process_yield() {
    if (runningTask != NULL) {
        runningTask->reschedule = true;
    }
    arch_enableInterrupts();
	asm volatile("int $32");
}
void process_schedule(registers_t *stack) {
	// send end of interrupt to interrupt controller
    // Simple, just find a task then switch to it.
    // Find any task that need to be killed
    if (disableScheduler) return;
    // This killing and waiting tasks handling code need to be more optimized
    struct process *kill = process_findByStatus(PROCESS_KILLING);
    if (kill != NULL) {
            if (runningTask == kill) {
                // Switch to kernel directory
                runningTask = idle;
            }   
        __process_destroy(kill);
    }
    struct process *waiting = process_findByStatus(PROCESS_WAITING);
    if (waiting != NULL) {
        if (waiting->wait_time-- == 0) {
            waiting->state = PROCESS_RUNNING;
            runningTask->quota = PROCESS_QUOTA;
            write_serialString("proc: waiting time reseted\r\n");
        }
    }
    struct process *nextTask = NULL;
    if (runningTask != NULL) {
        if (runningTask->lAddr == 0) {
            nextTask = NULL;
        } else {
            if (runningTask->quota < PROCESS_QUOTA && !runningTask->reschedule) {
                runningTask->quota++;
                return;
            }
            // Simply select new task
            nextTask = process_findNextByStatus(PROCESS_RUNNING,runningTask->lAddr);
            nextTask->quota = 0;
            nextTask->reschedule = false;
        }
    } else {
        // first switch
        nextTask = process_findByStatus(PROCESS_RUNNING);
    }
    if (nextTask == NULL) {
        // okay switch to idle
        nextTask = idle;
    }
    //if (nextTask == runningTask) return;
    // change the TSS and VMM
    tss_set_stack(0x10,nextTask->kernelESP);
    if (nextTask->dir == 0) {
        printf("proc: task %s directory are null, WTF?\n",nextTask->name);
        return;
    }
    vmm_switch((int *)nextTask->dir);
    arch_switchContext(nextTask);
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
    if (runningTask->pid == pid) {
        // Beause we use the quota, set runningTask to idle
        runningTask = idle;
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
    struct process *task = process_findID(pid);
    if (task == NULL || task->wait_time > 0) {
        printf("process_wait: bad call, type: %s, %x\n",(task == NULL ? "TYPE_INVALID_PARAMETER" : "TYPE_INVALID_TASK_WAIT_TIME"),(task == NULL ? 0 : task->wait_time));
        return; // bad call
    }
	task->wait_time = ms/10;
    task->state = PROCESS_WAITING;
    task->quota = PROCESS_QUOTA;
    process_yield();
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
        vmm_switch((int*)idle->dir);
		struct process *parent = process_findID(task->parent);
        if (parent == NULL) {
                printf("W: parent %d of %d null, reseting running task!\n",task->parent,task->pid);
                runningTask = NULL;
        }
	if (parent->state == PROCESS_WAITPID) {
		process_unblock(parent->pid);
	}
	curTasks--;
        if (task->arch_task != 0) {
            arch_destroyStack(task->arch_task);
            pmml_free(task->arch_task);
        }
	pmml_free(task->esp);
	/*
         * We don't free the memory, because i have bug here
         * so it's need to be rewriten and will be enabled
         * Sergij Yevchuk - Main Developer
         */
        for (int i = 0; i < task->pages; i++) {
            pmml_free((void *)task->page_start+(i*496));
        }
	/*
		Okay since i add FD support and it's need to be closed via sys_close syscall handler
	*/
		//clist_find(task->fds,process_closeDispatcher);
	pmml_free(task->fds);
	// now free the clist head structure
	if (task->user) {
            	pmml_free((void *)task->dir);
        }
	if (task->kernelESP != 0) {
		pmml_free((void *)task->kernelESP);
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
