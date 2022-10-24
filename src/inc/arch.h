#ifndef ARCH_H
#define ARCH_H
#include <process.h>
#include<typedefs.h>
#include<terminal.h>
#define PANIC(msg) panic(__FILE__,__func__,msg)
void arch_init(struct multiboot_info *info);
void arch_reset();
void arch_poweroff();
void panic(char *file,const char *funcName,const char *msg);
void arch_jumpToUser(int entryPoint,int userstack);
void arch_disableIRQ();
void arch_enableIRQ();
void *arch_buildStack(int entryPoint,bool isUserTask,int argc,char **argv);
void arch_disableInterrupts();
void arch_enableInterrupts();
void kassert(const char *file,const char *func,int line,bool exc);
#define assert(b) kassert(__FILE__,__func__,__LINE__,b)
// Copy arguments to process stack, architecture dependies
void arch_copy_process_args(struct process *p,int argc,char **argv,char **envp);
/* Destroy our stack, warrning: we are passing process ESP field as stack
@param stack Our stack to destroy
*/
void arch_destroyStack(void *stack);
/*
    Switch betwen task A and B
    @param stack Stack change to we need
*/
void arch_switchContext(void *newStack) __attribute__((cdecl));
void arch_switchToUser();
void arch_set_active_thread(struct process *active);
#endif
