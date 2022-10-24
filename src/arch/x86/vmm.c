/* X86 specific VMM code */
#include <mm/vmm.h>
#include <arch.h>
#include <mm/pmm.h>
#include <interrupts.h>
#include <terminal.h>
#include<mm.h>
#include<serial.h>
#include<x86/vmm_x86.h>
int kdirectory[1024] __attribute__((aligned(4096))); 
struct page_table_entry_t kpage_table[1024] __attribute__((aligned(4096)));
int pdir_3gb[1024] __attribute__((aligned(4096)));
int table[1024] __attribute__((aligned(4096)));
int *current_dir;
void vmm_init() {
	//memset(&kdirectory,0,sizeof(kdirectory));
   for (int i = 0; i < 1024; i++) {
    kpage_table[i].zero = 0;
    kpage_table[i].accessed = 0;
    kpage_table[i].available = 0;
    kpage_table[i].cache_disabled = 0;
    kpage_table[i].dirty = 0;
    kpage_table[i].global = 1;
    kpage_table[i].present = 1; /* kernel pages always in memory */
    kpage_table[i].read_write = 1; /* read & write */
    kpage_table[i].user_supervisor = 1; /* kernel mode pages */
    kpage_table[i].write_through = 1;
    kpage_table[i].page_phys_addr = (i * 4096) >> 12; /* assume 4Kb pages */
   }
   // fill 3gb dir
   for (int i=0, frame=0x100000, virt=0xc0000000; i<1024; i++, frame+=4096, virt+=4096) {
    pdir_3gb[PAGE_TABLE_INDEX(virt)] = (frame) | 7;
   }
   #ifndef LEGACY_TERMINAL
   for (int i = terminal_getBufferAddress(); i < (terminal_getBufferAddress()+terminal_getBufferSize())+4096; i+=4096) {
    table[PAGE_TABLE_INDEX(i)] = (i) | 7;
   }
   #endif
   kdirectory[0] = ((unsigned int)kpage_table) | 0x7;
   kdirectory[PAGE_DIRECTORY_INDEX(0xc0000000)] = ((unsigned int)pdir_3gb) | 7;
   #ifndef LEGACY_TERMINAL
   kdirectory[PAGE_DIRECTORY_INDEX(terminal_getBufferAddress())] = ((unsigned int)table) | 7;
   #endif
   interrupts_addHandler(14,vmm_pfault);
   current_dir = kdirectory;
#ifdef DEBUG
   write_serialString("vmm_x86: initialized\r\n");
#endif
 }
void vmm_pfault(registers_t *regs) {
    uint32_t addr = 0;
    uint32_t cr3 = 0;
    asm volatile("mov %%cr2, %0" : "=r"(addr));
    asm volatile("mov %%cr3, %0" : "=r"(cr3));
    if (process_getProcess(process_getCurrentPID())->user) {
        printf("Segmentation fault. at %x\n",addr);
        process_kill(process_getCurrentPID());
        arch_enableInterrupts();
        process_yield();
    }
  bool present = !(regs->error_code & 0x1);
  bool rw = regs->error_code & 0x2;
  bool us = regs->error_code & 0x4;
  bool reserved = regs->error_code & 0x8;
  bool fetch = regs->error_code & 0x10;
  printf("Page fault at %x, detailed info: %x\n",addr,regs->error_code);
  printf("Present: %d\nRW: %d\nUser: %d\nReserved: %d\nFetch: %d\n",present,rw,us,reserved,fetch);
  printf("Current directory address: %x\nRegistered address: %x\n",cr3,current_dir);
  //process_dump(process_getProcess(process_getCurrentPID()));
  while(1) {}
}
void vmm_enable() {
   uint32_t cr0;
   asm volatile("mov %%cr0, %0": "=r"(cr0));
   cr0 |= 0x80000000; // Enable paging!
   asm volatile("mov %0, %%cr0":: "r"(cr0));
}
void vmm_load() {
  asm volatile("movl %0, %%cr3":: "r"(current_dir));
}
void vmm_map(int *ptable,int phys,int virt) {
  ptable[PAGE_TABLE_INDEX(virt)] = (phys) | 7;
}
void vmm_dump(int *tabble) {
  for (int i = 0; i < 1024; i++) {
    int entry = tabble[i];
    if ((entry & 1) == 1) {
      printf("Index %d are present, dumping:\n",i);
      int *table = (int *)(entry & ~0x7);
      printf("%x - ",(table[0]) & ~0x7);
      int lastIndex = 1;
      for (int i = 1; i < 1024; i++) {
        if ((table[i] & 1) == 1) {
          lastIndex = i;
        }
      }
      printf("%d, %x\n",lastIndex,(table[lastIndex]) & ~0x7);
    }
  }
}
void vmm_switch(int *dir) {
  if (dir == NULL) return;
  if (current_dir == dir) {
      return;
  }
  current_dir = dir;
  vmm_load();
}
int *vmm_allocTable(int *table,int addr) {
  int entry = table[PAGE_DIRECTORY_INDEX(addr)];
  if (!(entry & 1)) {
    int *tablee = (int *)pmml_alloc(true);
    tablee[PAGE_DIRECTORY_INDEX(addr)] = ((unsigned int)tablee) | 7;
    return tablee;
  }
  printf("%s: returning already allocated table\n",__func__);
  return (int *)(entry & ~7);
}
int *vmm_getCurrentDirectory() {
  return current_dir;
}
int *vmm_createDirectory() {
  /* Clone the kernel directory then return */
  int *directory = pmml_allocPages(2,true);
  int *bios = pmml_allocPages(2,true);
  int *gb3 = pmml_allocPages(2,true);
  int *frameBuffer = pmml_allocPages(2,true);
  if (directory == NULL || bios == NULL || gb3 == NULL || frameBuffer == NULL) {
    PANIC("vmm_createTable: NuLL");
  }
  for (int i = 0,frame=0x0, virt=0x00000000; i < 1024; i++,frame+=4096, virt+=4096) {
    bios[PAGE_TABLE_INDEX(virt)] = (frame) | 7;
  }
  for (int i=0, frame=0x100000, virt=0xc0000000; i<1024; i++, frame+=4096, virt+=4096) {
    gb3[PAGE_TABLE_INDEX(virt)] = (frame) | 7;
   }
   #ifndef LEGACY_TERMINAL
   for (int i = terminal_getBufferAddress(); i < (terminal_getBufferAddress()+terminal_getBufferSize())+4096; i+=4096) {
    frameBuffer[PAGE_TABLE_INDEX(i)] = (i) | 7;
   }
   directory[PAGE_DIRECTORY_INDEX(terminal_getBufferAddress())] = ((unsigned int)frameBuffer) | 7;
   #endif
   /* Fill table with the specific data */
   directory[0] = ((unsigned int)bios) | 7;
   directory[PAGE_DIRECTORY_INDEX(0xc0000000)] = ((unsigned int)gb3) | 7;
   /* Now return */
   return directory;
}
int vmm_translate(int *ptable,int virtualAddress) {
  int index = PAGE_DIRECTORY_INDEX(virtualAddress);
  int table_index = (virtualAddress >> 12) & 0x7;
  if (index > 1024) {
    printf("%s: index overflow\n",__func__);
    return 0;
  }
  if (table_index > 1024) {
    printf("%s: page table index overflow\n",__func__);
    return 0;
  }
  if (ptable[index] == 0) {
    printf("%s: we don't map this in page directory\n",__func__);
    /* We don't map this */
    return 0;
  }
  int *table = (int *)PAGE_GET_PHYSICAL_ADDRESS(&ptable[index]);
  int enAddr = table[table_index] & ~0xfff;
  /* Here we need to return correctly address using "align", for example the virtual address are 0x400176,
     and it's physical address are 0x16c176, but without align we return 0x16c00 */
  int align = virtualAddress - (index << 22);
  return enAddr + align;
}
