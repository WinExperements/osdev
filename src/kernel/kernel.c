/* HelinOS - an operation system  */
/* This file contains our entry point after CPU bootstrap code, just do some
architecture initalization, then load and execute buid-in modules in the 
kernel, after that try to read and execute the init program from the
CDROM or other boot media */
/* Keyboard internal test */
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
#include<initrd.h>
#include<symbols.h>
#include<kshell.h>
#include<lib/clist.h>
extern char kernel_end[];
extern char kernel_start[];
bool verbose;
typedef void entry_t();
void log_status(char *component,bool status);
extern void kernel_main(struct multiboot_info *multiboot) {
	printf("Helin OS kernel 0.0.8 - Initrd and VFS\n");
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
		//PANIC("Must be at less one module(initrd) passed.");
	}
	log_status("PMM",true);
	vmm_init();
	log_status("VMM init",true);
	arch_disableIRQ();
	process_init();
	log_status("Process Manager",true);
	//clist_test();
	arch_enableIRQ();
	vmm_load();
	vmm_enable();
	log_status("VMM enable",true);
	keyboard_init();
	log_status("Keyboard",true);
	atapi_init();
	log_status("ATAPI",true);
	vfs_init();
	log_status("VFS",true);
	rootfs_init();
	rootfs_mount("/");
	vfs_creat(vfs_getRoot(),"dev",VFS_DIRECTORY);
	vfs_creat(vfs_getRoot(),"bin",VFS_DIRECTORY);
	log_status("rootfs",true);
	dev_init();
	log_status("Device manager",true);
	tty_init();
	log_status("TTY",true);
	kshell_init(multiboot);
	char *cmdline = (char *)multiboot->cmdline;
	if (strcmp(cmdline,"-v")) {
		verbose = true;
	}
	if (strcmp(cmdline,"-nokshell")) {
		multiboot_module_t *mod = (multiboot_module_t *)multiboot->mods_addr;
		elf_load_file((void *)mod->mod_start,true,"init");
	} else {
		process_create((int)kshell_main,false,"kshell");
		/*process_create((int)kshell_main,false,"kshefll");*/
	}
	arch_enableInterrupts();
	arch_switchToUser();
}
void log_status(char *component,bool status) {
	printf("[%s] %s\n",(status ? "OK" : "FAIL"),component);
}