#ifndef STDLIB_H
#define STDLIB_H
int getpid();
void exit(int exitcode);
void *malloc(int size);
void free(void *p,int pages);
#endif
