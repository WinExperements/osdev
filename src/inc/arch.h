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
void arch_switchToUser();
void arch_disableIRQ();
void arch_enableIRQ();
void *arch_buildStack(int entryPoint,bool isUserTask);
void arch_disableInterrupts();
void arch_enableInterrupts();
void kassert(const char *file,const char *func,int line,bool exc);
#define assert(b) kassert(__FILE__,__func__,__LINE__,b)
#endif
