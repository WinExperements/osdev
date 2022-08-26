#ifndef PROCESS_H
#define PROCESS_H
#include<typedefs.h>
#include<mm/vmm.h>
#include <arch.h>
#include<ipc/message.h>
#define STATE_READY 2
struct _stackFrame {
  uint32_t gs;
  uint32_t fs;
  uint32_t es;
  uint32_t ds;
  uint32_t eax;
  uint32_t ebx;
  uint32_t ecx;
  uint32_t edx;
  uint32_t esi;
  uint32_t edi;
  uint32_t esp;
  uint32_t ebp;
  uint32_t eip;
  uint32_t cs;
  uint32_t flags;
  uint32_t usersp;
  uint32_t ss;
};
typedef struct _stackFrame stackFrame;
struct process {
	void *esp;
	struct process *next;
	bool used;
	int pid;
  char *name;
  int kernelESP;
  uint32_t dir;
  int state;
  int pages;
  msg_t messages[10];
  int message_count;
  int wait_time;
  int parent;
  struct process *child;
  int lAddr;
};
extern struct process *runningTask;
void process_init();
struct process *process_create(int,bool isUser,char *name);
void process_delete(struct process *);
void process_kill(int);
void process_dump(struct process *);
/* process_schedule - Process scheduler function
stack - Current Stack
Return Value:
Next task stack
 */
void *process_schedule(void *stack);
int process_getCurrentPID();
void process_exit(int pid,int returnCode);
void process_switchIdle();
struct process *process_getProcess(int pid);
void process_update(int pid,struct process *to);
void process_block(int pid);
void process_unblock(int pid);
void process_wait(int pid,int ms);
/* Wait for any child process if the pid are -1 */
void process_waitPid(int pid);
void process_yield();
#endif
