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
static void print_logo() {
	printf(" _    _      _ _        ____   _____ \n"); 
 	printf("| |  | |    | (_)      / __ \\ / ____| \n");
 	printf("| |__| | ___| |_ _ __ | |  | | (___   \n");
 	printf("|  __  |/ _ \\ | | '_ \\| |  | |\\___ \\ \n");
 	printf("| |  | |  __/ | | | | | |__| |____) |    \n");
 	printf("|_|  |_|\\___|_|_|_| |_|\\____/|_____/ \n");
        printf("                                     \n");                                
}
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
    _multiboot = multiboot; 
    	print_logo();
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
    symbols_init(multiboot);
	vmm_init();
	vfs_init();
	rootfs_init();
	rootfs_mount("/");
	vfs_creat(vfs_getRoot(),"dev",VFS_DIRECTORY);
	vfs_creat(vfs_getRoot(),"bin",VFS_DIRECTORY);
	vfs_creat(vfs_getRoot(),"initrd",VFS_DIRECTORY);
	arch_disableIRQ();
	process_init();
	arch_enableIRQ();
	vmm_load();
	vmm_enable();
	keyboard_init();
	//atapi_init();
	dev_init();
	tty_init();
	kshell_init(multiboot);
	//printf("CHECK IF THE MEMORY PROPERTLY MAPPED!\n");
	char *cmdline = (char *)multiboot->cmdline;
	parse_commandLine(cmdline);
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
					int name_size = strlen((char *)mod->cmdline)-4;
					if (strcmp((char *)mod->cmdline+name_size,".mod")) {
						module_t *modu = load_module((void *)mod->mod_start);
						if ((modu == NULL) || (!modu->init)) {
							printf("Module broken, see console\n");
						} else {
							modu->init(modu);
						}
					}
        	}
    	}
    	if (!disableStartInit) {
        	if (!exec_init()) {
            		printf("Failed, entering kshell\n");
            		process_create((int)kshell_main,false,"kshell",0,NULL);
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
