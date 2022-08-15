#include "stdlib.h"
int getpid() {
	int res = 0;
	asm volatile("int $0x80" : "=a" (res) : "0" (4));
	return res;
}
void exit(int exitcode) {
	int res = 0;
	asm volatile("int $0x80" : "=a"(res) : "0" (2));
}
