#ifndef MM_H
#define MM_H
#include "typedefs.h"
typedef struct chunk_t {
	struct chunk_t *next;
	struct chunk_t *prev;
	int alloc;
	u32 size;
} chunk_t;
void *memset(void *,char,int);
void mm_init(u32,u32);
void *kmalloc(u32);
void kfree(void *);
#endif
