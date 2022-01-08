#include "kernel.h"
void panic(char *msg) {
	printk("Kernel panic:");
	printk(msg);
	printk("\n");
	for (;;) {}
}