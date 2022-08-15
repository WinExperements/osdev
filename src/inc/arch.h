#ifndef ARCH_H
#define ARCH_H
#include <process.h>
#include<typedefs.h>
#define PANIC(msg) panic(__FILE__,__func__,msg)
void arch_init(struct multiboot_info *info);
void arch_reset();
void arch_poweroff();
void panic(char *file,const char *funcName,const char *msg);
void arch_switchToUser();
void arch_disableIRQ();
void arch_enableIRQ();
void *arch_buildStack(int entryPoint,bool isUserTask);
#endif
