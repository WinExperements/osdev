#include "stdlib.h"
#include "syscall.h"
int getpid() {
	return helin_syscall(4,0,0,0,0,0);
}
void exit(int exitcode) {
	helin_syscall(2,exitcode,0,0,0,0);
	for (;;) {}
}
void *malloc(int size) {
	return (void *)helin_syscall(11,(size/4096)+1,0,0,0,0);
}
/* Need to be fixed and need to be added in program page tracking */
void free(void *address,int pages) {
	helin_syscall(12,(int)address,pages,0,0,0);
}