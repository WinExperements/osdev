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
#include<vfs.h>
#include<initrd.h>
#include<symbols.h>
#include<kshell.h>
#include<lib/clist.h>
#include<stddef.h>
#include<symbols.h>
extern char kernel_end[];
extern char kernel_start[];
bool verbose;
bool disableStartInit;
multiboot_info_t *_multiboot;
void print_symbols();
void parse_commandLine(char *commandline) {
    char *k = strtok(commandline," ");
    while(k != NULL) {
        if (strcmp(k,"-noinit")) {
            disableStartInit = true;
        }
        k = strtok(NULL," ");
    }
}
extern void kernel_main(struct multiboot_info *multiboot) {
	printf("Helin OS kernel compiled at %s:%s ",__DATE__,__TIME__);
    _multiboot = multiboot; 
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
	printf("with %d MB memory\n",(pmml_getMaxBlocks()*4096)/1024/1024);
    	symbols_init(multiboot);
	printf("Initializing VMM and VFS\n");
	vmm_init();
	vfs_init();
	rootfs_init();
	rootfs_mount("/");
	vfs_creat(vfs_getRoot(),"dev",VFS_DIRECTORY);
	vfs_creat(vfs_getRoot(),"bin",VFS_DIRECTORY);
	vfs_creat(vfs_getRoot(),"initrd",VFS_DIRECTORY);
    	printf("Initializing process manager\n");
	arch_disableIRQ();
	process_init();
	arch_enableIRQ();
    	printf("Enabling VMM and Initializing keyboard\n");
	vmm_load();
	vmm_enable();
	keyboard_init();
	//atapi_init();
   	 printf("Initializing devfs and tty's\n");
	dev_init();
	tty_init();
    	printf("Initializing kshell\n");
	kshell_init(multiboot);
    	printf("Parsing command line\n");
	char *cmdline = (char *)multiboot->cmdline;
	parse_commandLine(cmdline);
    	printf("Filling ramfs with modules\n");
    	vfs_node_t *dev = vfs_finddir(vfs_getRoot(),"bin");
    	if (!dev) {
        	printf("no /bin found, execution terminated\n");
        	return;
    	} else if ((dev->flags & 0x7) != VFS_DIRECTORY) {
        	printf("/bin not a directory, execution terminated\n");
		return;
    	}
   	 for (int i = 0; i < multiboot->mods_count; i++) {
        	multiboot_module_t *mod = (multiboot_module_t *)multiboot->mods_addr+i;
       		if (mod->cmdline != 0) {
            		vfs_node_t *in = vfs_creat(dev,(char *)mod->cmdline,0);
            		rootfs_insertModuleData(in,mod->mod_end-mod->mod_start,(char *)mod->mod_start);
        	}
    	}
    	printf("Finishing up\n");
    	if (!disableStartInit) {
        	printf("Starting init\n");
        	if (!exec_init()) {
            		printf("Failed, entering kshell\n");
            		process_create((int)kshell_main,false,"kshell",0,NULL);
        	} else {
            		printf("Init created\n");
        	}
    	} else {
        	process_create((int)kshell_main,false,"kshell",0,NULL);
    	}
    	arch_enableInterrupts();
}
bool exec_init() {
	if (_multiboot->mods_count > 0) {
		vfs_node_t *n = vfs_find("/bin/init");
		if (!n) {
			printf("/bin/init file not found\n");
			return false;
		}
		int pages = (n->size/4096)+1;
		void *start = pmml_allocPages(pages,true);
		vfs_read(n,0,n->size,start);
		char **argv = pmml_alloc(true);
		argv[0] = "/bin/init";
		argv[1] = "init";
       		/* char **envp = pmml_alloc(true);
        	envp[0] = "PATH=/bin";*/
		if (elf_load_file(start,true,"init",1,argv,NULL)) {
            		// okay it must 1
           		struct process *init = process_getProcess(process_getProcesses()-1);
           		init->parent = 0;
           		init->pid = 1;
			pmml_freePages(start,pages-1);
			return true;
		} else {
			pmml_free(argv);
			pmml_freePages(start,pages-1);
		}
	}
	return false;
}
