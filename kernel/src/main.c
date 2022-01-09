#include "printk.h"
#include "kernel.h"
#include "arch.h"
#include "mm.h"
typedef void (*constructor)();
constructor start_ctors;
constructor end_ctors;
extern void callConstructors() {
	for(constructor* i = &start_ctors; i != &end_ctors; i++)
        (*i)();
}
extern void kmain(struct multiboot_info *info) {
	arch_setup();
	printk("Helin kernel version 0.1, booted on x86 CPU\n");
	printk("Memory low: ");
	printkDec(info->low_mem);
	printk("\nMemory high: ");
	printkDec(info->high_mem);
	mm_init(info->low_mem/1024,(info->high_mem-info->low_mem)*1024);
        char *arr = kmalloc(1024);
	if (arr != 0) {
		printk("Test passed, now freeing allocated pages!\n");
		kfree(arr);
	} else {
		panic("TEST FAIL!\n");
	}
}
void kernel_shutdown() {
	printk("Kernel shutdown in progress!\n");
	panic("The architecture specific function are not works, please shutdown manually");
}
