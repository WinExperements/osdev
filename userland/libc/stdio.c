#include "stdio.h"
/* Print string on screen */
void printf(char *message) {
	int res = 0;
	asm volatile("int $0x80" :"=a" (res) : "0" (1), "b" ((int)message));
}
