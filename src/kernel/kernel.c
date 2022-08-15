/* HelinOS - an operation system  */
/* This file contains our entry point after CPU bootstrap code, just do some
architecture initalization, then load and execute buid-in modules in the 
kernel, after that try to read and execute the init program from the
CDROM or other boot media */
#include<terminal.h>
#include<serial.h>
#include<arch.h>
#include<mm.h>
#include<mm/pmm.h>
#include<process.h>
#include <mm/vmm.h>
#include <syscall.h>
#include <keyboard.h>
#include <io.h>
#include <interrupts.h>
#include <x86/gdt.h>
#include <mstring.h>
#include<elf.h>
#include<dev.h>
#include <atapi/atapi.h>
#include<vfs.h>
extern char kernel_end[];
extern char kernel_start[];
bool verbose;
typedef void entry_t();
extern void kernel_main(struct multiboot_info *multiboot) {
	printf("Helin OS kernel 0.0.5 - Clock wait\n");
	if (!(multiboot->flags >> 6 & 0x1))
	{
		PANIC("No memory map");
	}
	if (!(multiboot->flags >> 6 & 0x3)) {
		PANIC("No modules provided from bootloader, are you booted from GRUB?");
	}
	/* If modules are passed, then init PMM using the last module end address */
	if (multiboot->mods_count != 0) {
		uint32_t new_addr = ((multiboot_module_t *)multiboot->mods_addr+(multiboot->mods_count-1))->mod_end;
		uint32_t new_size = new_addr-(int)kernel_start;
		ppml_init(multiboot,new_addr,new_size);
	} else {
		ppml_init(multiboot,(uint32_t)kernel_end,(int)kernel_end-(int)kernel_start);
	}
	vmm_init();
	arch_disableIRQ();
	process_init();
	arch_enableIRQ();
	vmm_load();
	vmm_enable();
	keyboard_init();
	atapi_init();
	vfs_init();
	if (strcmp((char *)multiboot->cmdline,"-v")) {
		verbose = true;
	}
	for (int i = 0; i < multiboot->mods_count; i++) {
		multiboot_module_t *module = (multiboot_module_t *)multiboot->mods_addr+i;
		/* Before any loading check if it's module are our kernel driver */
		bool isUser = true;
		if (module->cmdline != 0 && strcmp((char *)module->cmdline,"-drv")) {
			isUser = false;
		}
		elf_load_file((void *)module->mod_start,isUser);
	}
	asm volatile("sti");
	arch_switchToUser();
}
