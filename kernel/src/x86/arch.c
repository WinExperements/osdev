#include "arch.h"
#include "printk.h"
#include "arch/x86/gdt.h"
#include "interrupt.h"
void arch_setup() {
	printk_init();
	printk_clear();
	gdt_init();
    intr_init();
	printk("arch: set-up complete\n");
}
