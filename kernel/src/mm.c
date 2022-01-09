#include "mm.h"
chunk_t *mem;
void *memset(void *dest,char val,int count) {
	char *temp = (char *)dest;
  for( ; count != 0; count--) *temp++ = val;
  return dest;
}
void mm_init(u32 start,u32 size) {
	if (size < sizeof(chunk_t)) {
		mem = 0;
	} else {
		mem = (chunk_t *)start;
		mem->alloc = 0;
		mem->size = size-sizeof(chunk_t);
	}
}
void *kmalloc(u32 size) {
	chunk_t *result = 0;
	for (chunk_t *chunk = mem; chunk != 0 && result == 0; chunk = chunk->next) {
		if(chunk->size > size && chunk->alloc == 0) {
            result = chunk;
        }
   }
   if (result == 0) {
   	//Failed to find free memory chunk
   	return 0;
  }
  if (result->size >= size + sizeof(chunk_t)+1) {
  	chunk_t *tmp = (chunk_t *)((u32)result+sizeof(chunk_t)+size);
      tmp->alloc = 0;
      tmp->size = result->size - size - sizeof(chunk_t);
      tmp->prev = result;
      tmp->next = result->next;
      if (tmp->next != 0) {
      	tmp->next->prev = tmp;
      }
      result->size = size;
      result->next = tmp;
  }
  result->alloc = 1;
  return (void *)(((u32)result) + sizeof(chunk_t));
}
void kfree(void *ptr) {
	chunk_t* chunk = (chunk_t*)((u32)ptr - sizeof(chunk_t));
    chunk -> alloc= 0;
    if(chunk->prev != 0 && chunk->prev->alloc != 1)
    {
        chunk->prev->next = chunk->next;
        chunk->prev->size += chunk->size + sizeof(chunk_t);
        if(chunk->next != 0)
            chunk->next->prev = chunk->prev;
        
        chunk = chunk->prev;
    }
    
    if(chunk->next != 0 && !chunk->next->alloc)
    {
        chunk->size += chunk->next->size + sizeof(chunk_t);
        chunk->next = chunk->next->next;
        if(chunk->next != 0)
            chunk->next->prev = chunk;
    }
}
