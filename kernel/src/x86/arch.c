#include "arch.h"
#include "printk.h"
#include "arch/x86/gdt.h"
#include "interrupt.h"
#include "timer.h"
void arch_setup() {
	printk_init();
	printk_clear();
	printk("WARRNING THIS BUILD IS A DEBUG NOT RELEASE!\n");
	printk("Initializing GDT...");
	gdt_init();
	printk("ok\nInitializing interrupts...");
        intr_init();
	printk("ok\nInitializing timer...");
	timer_init(100); 
	printk("ok\nset-up complete\n");
}
