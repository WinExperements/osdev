#ifndef PROCESS_H
#define PROCESS_H
#include<typedefs.h>
#include<mm/vmm.h>
#include<ipc/message.h>
#include<vfs.h>
#include<lib/clist.h>
#define PROCESS_KILLING 1
#define PROCESS_WAITPID 2 // process waiting for childs
#define PROCESS_QUOTA 10
#define PROCESS_WAITING 3
typedef struct _stackFrame stackFrame;
struct process {
    /* Process information */
	void *esp;
	struct process *next;
    /* Scheduler information */
	bool used;
	int pid;
    char *name;
    /* Context information */
    int kernelESP;
    uint32_t dir;
    /* Other information about the process */
    int state;
    int pages;
    msg_t messages[10];
    int message_count;
    int wait_time;
    int parent;
    struct process *child;
    int lAddr;
    vfs_node_t *workDir;
    int page_start;
    int quota;
    bool user;
    bool reschedule;
    void *arch_task; // Architecture dependes task structure
    int uid;
    int guid;
};
extern struct process *runningTask;
/* 
 Initialize the process manager(scheduler)
 */
void process_init();
/*
 * Creates new task
 @param entry Entry point
 @param isUser - Is task must runned in user mode
 @param name Task name
*/
struct process *process_create(int entry,bool isUser,char *name,int argc,char **argv);
/*
 * Delete the task from process list
*/
void process_delete(struct process *);
/*
 * Kill task by it's own PID
*/
void process_kill(int);
/*
 * Dump process
*/
void process_dump(struct process *);
/* process_schedule - Process scheduler function
stack - Current Stack
Return Value:
Next task stack
 */
void process_schedule();
/*
 * Return current process ID
*/
int process_getCurrentPID();
/*
 * Exit from currently process, by id and return code
 * called by process_kill
 * TODO: Re-write it and remove calling exit for kill
*/
void process_exit(int pid,int returnCode);
/*
 * Return process structure by PID
*/
struct process *process_getProcess(int pid);
/*
 * Block process from execution
*/
void process_block(int pid);
/*
 * Unblock process and re-schedule
*/
void process_unblock(int pid);
/* 
 * Process sleep and wait function implemented here
*/
void process_wait(int pid,int ms);
/* Wait for any child process if the pid are -1 */
void process_waitPid(int pid);
/*
 * Re-schedule
*/
void process_yield();
/*
 * Return total count of processes
*/
int process_getProcesses();
/*
 * Get our task list
*/
void process_dumpAll();
/*
 * Disable scheduler
*/
void process_disableScheduler();
/*
 * Enable scheduler
*/
void process_enableScheduler();
/*
 * Return current PID index, mostly used for exec syscall
*/
int process_getNextPID();
#endif
