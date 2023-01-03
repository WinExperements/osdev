#ifndef MM_PMM_H
#define MM_PMM_H
#define PHYS_PAGE_SIZE 4096
#include<typedefs.h>
void ppml_init(struct multiboot_info *info,uint32_t endkernel,uint32_t kernel_size);
void *pmml_alloc(bool);
int pmml_free(void *addr);
int pmml_getMemorySize();
int pmml_getMaxBlocks();
void *pmml_allocPages(int pages,bool clear);
void pmml_freePages(void *address,int count);
void pmml_initRegion(uint32_t address,uint32_t size);
void pmml_deinit_region(uint32_t address,uint32_t size);
/* Returns count of free blocks of memory */
int pmml_getFreeBlocks();
bool pmml_isPageAllocated(void *page);
/* Function that must be used to map all memory! */
int pmml_getMemStart(); // Return start address of memory
int pmml_getMemEnd(); // Return size of detected memory
#endif
