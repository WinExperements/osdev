#include "printk.h"
#include "kernel.h"
#include "arch.h"
typedef void (*constructor)();
constructor start_ctors;
constructor end_ctors;
extern void callConstructors() {
	for(constructor* i = &start_ctors; i != &end_ctors; i++)
        (*i)();
}
extern void kmain(const void *multiboot,u32 magic) {
	arch_setup();
	printk("Helin kernel version 0.1, booted on x86 CPU\n");
	kernel_shutdown();
}
void kernel_shutdown() {
	printk("Kernel shutdown in progress!\n");
	panic("The architecture specific function are not works, please shutdown manually");
}
