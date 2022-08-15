#ifndef MM_VMM_H
#define MM_VMM_H
#include<typedefs.h>
#include <x86/idt.h>
void vmm_init();
void vmm_enable();
void vmm_pfault(registers_t *regs);
void vmm_load();
void vmm_map(int *ptable,int phys,int virt);
void vmm_dump(int *table);
void vmm_switch(int *dir);
int *vmm_allocTable(int *table,int address);
int *vmm_getCurrentDirectory();
int *vmm_createDirectory();
int vmm_translate(int *ptable,int virtualAddress);
#endif
