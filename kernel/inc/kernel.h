#ifndef KERNEL_H
#define KERNEL_H
#include "printk.h"
#include "typedefs.h"
void panic(char *msg);
extern void kmain(const void *,u32);
void kernel_shutdown();
#endif