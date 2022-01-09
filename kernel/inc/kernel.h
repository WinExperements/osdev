#ifndef KERNEL_H
#define KERNEL_H
#include "printk.h"
#include "typedefs.h"
void panic(char *msg);
extern void kmain(struct muliboot_info *);
void kernel_shutdown();
#endif
