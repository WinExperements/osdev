/** Physical Page Allocator  **/

#include<mm/pmm.h>
#include<mm.h>
#include<serial.h>
#include<arch.h>
#include<terminal.h>
uint32_t main_memory_size;
uint16_t *bitmap;
uint32_t max_blocks,free_blocks,used_blocks;
void mmap_set(int index,bool set) {
	if (set) {
		bitmap[index] = 255;
	} else {
		bitmap[index] = 0;
	}
}
bool mmap_test(int index) {
	if (bitmap[index] == 255) {
		return true;
	} else {
		return false;
	}
}
int pmml_firstFree() {
	for (int i = 0; i < max_blocks; i++) {
		if (!mmap_test(i)) {
			return i;
		}
	} 
	return -1;
}
int pmml_firstFree_size(uint32_t size) {
	if (size == 0) {
		return -1;
	}
	if (size == 1) {
		return pmml_firstFree();
	}
	uint32_t frame = 0;
	for (int i = 0; i < max_blocks; i++) {
		if (!mmap_test(i)) {
			frame++;
		}
		if (frame == size) {
			return i;
		}
	}
	return -1;
}
void ppml_init(struct multiboot_info *info,uint32_t _endkernel,uint32_t kernel_size)
{
	bitmap = (uint16_t *)_endkernel;
	uint32_t size = 0;
	for (unsigned long i = 0; i < info->mmap_length; i+=sizeof(multiboot_memory_map_t)) {
		multiboot_memory_map_t *en = (multiboot_memory_map_t *)(info->mmap_addr+i);
		if (en->type == 1 && en->len_low > size) {
			size = en->len_low;
		}
	}
	size-=kernel_size;
	int bitmapSize = (size/4096)*sizeof(uint16_t);
	size-=bitmapSize;
	max_blocks = size / 4096;
	used_blocks = max_blocks;
	for (int i = 0; i < max_blocks; i++) {
		mmap_set(i,true);
	}
	pmml_initRegion(_endkernel+bitmapSize,size);
}
void *pmml_alloc(bool clear) {
	if (pmml_getFreeBlocks() <= 0) {
		printf("%s: no free blocks\n",__func__);
		return NULL;
	}
	int frame = pmml_firstFree();
	if (frame == -1) {
		printf("%s: Cannot found first free block, %x\n",__func__,frame);
		return NULL;
	}
	mmap_set(frame,true);
	uint32_t addr = frame * 4096;
	if (clear) {
		memset((void *)addr,0,4096);
	}
	return (void *)addr;
}
int pmml_free(void *addr)
{
	uint32_t add = (uint32_t)addr;
	int frame = add / 4096;
	mmap_set(frame,false);
	used_blocks++;
	return true;
}
int pmml_getMemorySize() {
	return main_memory_size/1024/1024;
}
int pmml_getMaxBlocks() {
	return max_blocks;
}
void *pmml_allocPages(int count,bool clear) {
	if (pmml_getFreeBlocks() >= count) {
		int frame = pmml_firstFree_size(count);
		if (frame == -1) {
			printf("%s: Cannot find any free blocks\n",__func__);
			return NULL;
		}
		for (int i = 0; i < count; i++) {
			mmap_set(frame+i,true);
		}
		uint32_t addr = frame*4096;
		used_blocks+=count;
		if (clear) {
			memset((void *)addr,0,count*4096);
		}
		return (void *)addr;
	} else {
		printf("%s: no free space left.\n",__func__);
	}
	return NULL;
}
void pmml_freePages(void *pageAddress,int count) {
	uint32_t addr = (uint32_t)pageAddress;
	int frame = addr/4096;
	for (int i = 0; i < count; i++) {
		mmap_set(frame+i,false);
	}
	used_blocks-=count;
}
void pmml_initRegion(uint32_t address,uint32_t size) {
	int align = address / 4096;
	int block = size / 4096;
	for (; block>0; block--) {
		mmap_set(align++,false);
		used_blocks--;
	}
	mmap_set(0,true);
}
int pmml_getFreeBlocks() {
	return max_blocks-used_blocks;
}