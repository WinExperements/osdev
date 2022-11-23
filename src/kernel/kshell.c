#include<keyboard.h>
#include<terminal.h>
#include<kshell.h>
#include<vfs.h>
#include<mstring.h>
#include<typedefs.h>
#include<initrd.h>
#include<mm/pmm.h>
#include<elf.h>
#include<io.h>
#include<lib/clist.h>
#include<symbols.h>
#include<arch.h>
void parseCommand(int argc,char *cmd[]);
multiboot_info_t *info;
vfs_node_t *kshell_tty;
int pid;
char path[128];
bool exit;
int syscall(int num,int p1,int p2,int p3,int p4,int p5) {
    // use asm macro for it
    int ret = 0;
    asm volatile("int %1\n" 
                : "=a" (ret) 
                : "i" (0x80),
                "a" (num),
                "d" (p1),
                "c" (p2),
                "b" (p3),
                "D" (p4),
                "S" (p5)
                : "cc", "memory");
    return ret;
}
void kshell_init(multiboot_info_t *i) {
    //printf("KSHELL: My pid: %d\n",process_getCurrentPID());
    info = i;
    vfs_node_t *dev = vfs_finddir(vfs_getRoot(),"dev");
    if (!dev) {
        printf("kshell: no /dev!\n");
        return;
    }
    kshell_tty = vfs_finddir(dev,"tty");
    if (!kshell_tty) {
        printf("Failed to find /dev/tty!\n");
        return;
    }
}
void getInput(char *buff,int how) {
    // let's use our syscall
    arch_enableInterrupts();
    vfs_read(kshell_tty,0,100,buff);
}
void kshell_main() {
    printf("Kernel debugger shell\n");
    exit = false;
    char buff[100];
    char *argv[100];
    char *name = process_getProcess(process_getCurrentPID())->name;
    while(!exit) {
        int argc = 0;
        vfs_node_path(process_getProcess(process_getCurrentPID())->workDir,path,128);
        printf("%s > ",name);
        getInput(buff,100);
        if (buff[0] == '1') continue;
        argv[argc] = strtok(buff," ");
        while(argv[argc]) {
            argc++;
            argv[argc] = strtok(NULL," ");
        }
        if (argc > 0) {
            parseCommand(argc,argv);
        }
    }
}
void parseCommand(int argc,char *cmd[]) {
    if (strcmp(cmd[0],"ls")) {
        if (!vfs_getRoot()) {
            printf("ls: no mount point to /\n");
        } else {
            vfs_node_t *in = vfs_find(path);
            if (argc > 1) {
                in = vfs_finddir(in,cmd[1]);
                if (!in){
                    printf("ls: %s: no such file or directory\n");
                }
            }
            struct dirent *d;
            int i = 0;
            while((d = vfs_readdir(in,i)) != 0) {
                printf("%d, %s\n",i,d->name);
                i++;
            }
        }
    } else if (strcmp(cmd[0],"mount")) {
        if (!info || info->mods_count == 0) {
            printf("mount: no initrd passed to mount\n");
        } else {
            multiboot_module_t *mod = (multiboot_info_t *)info->mods_addr;
            initrd_init((void *)mod->mod_start);
            initrd_mount("/");
        }
    } else if (strcmp(cmd[0],"cls")) {
        terminal_clear();
    } else if (strcmp(cmd[0],"exit")) {
        syscall(2,0,0,0,0,0);
        //exit = true;
    } else if (strcmp(cmd[0],"cat")) {
        if (argc > 1) {
            if (!vfs_getRoot()) {
                printf("cat: no mount point to /\n");
            } else {
                vfs_node_t *n = vfs_find(cmd[1]);
                if (!n) {
                    printf("cat: %s: no such file or directory\n",cmd[1]);
                } else if ((n->flags & 0x7) == VFS_DIRECTORY) {
                    printf("cat: %s: is a directory\n",cmd[1]);
                } else {
                    if (n->size != 0) {
			int f_size = (argc > 2 ? atoi(cmd[2]) : n->size);
                        int pages = (f_size/4096)+1;
			printf("Size: %d\n",f_size);
                        char *buf = pmml_allocPages(pages,true);
			if (!buf) {
				printf("cat: no space left to read\n");
				return;
			}
                        vfs_read(n,0,f_size,buf);
                        printf("%s\n",buf);
                        pmml_freePages(buf,pages);
                    }
                }
            }
        } else {
            printf("cat: require file name\n");
        }
    } else if (strcmp(cmd[0],"exec")) {
         if (argc > 1) {
            if (!vfs_getRoot()) {
                printf("exec: no mount point to /\n");
            } else {
                int ret = syscall(13,(int)cmd[1],0,0,0,0);
                if (ret > 0) {
                    syscall(22,ret,0,0,0,0);
                }
            }
        }
    } else if (strcmp(cmd[0],"reboot")) {
        syscall(14,0,0,0,0,0);
    } else if (strcmp(cmd[0],"cd")) {
        static vfs_node_t *prev = NULL;
        if (argc > 1) {
            if (strcmp(cmd[1],"..")) {
                if (!prev) {
                    // we don't change directory previest
                    printf("cd: no ..\n");
                    return;
                }
                syscall(17,(int)prev->name,0,0,0,0);
                return;
            }
            vfs_node_t *to = vfs_finddir(vfs_getRoot(),cmd[1]);
            if (!to) {
                printf("cd: %s: no such file or directory\n",cmd[1]);
                return;
            }
            if ((to->flags & 0x7) != VFS_DIRECTORY) {
                printf("cd: %s: not a directory\n",cmd[1]);
            }
            // save previest node if we need to switch to ..
            prev = (vfs_node_t *)syscall(16,0,0,0,0,0);
            syscall(17,(int)to->name,0,0,0,0);
        } else {
            printf("%s\n",vfs_getRoot()->name);
        }
    } else if (strcmp(cmd[0],"touch")) {
        if (argc > 1) {
            vfs_creat(vfs_getRoot(),cmd[1],0);
        }
    } else if (strcmp(cmd[0],"wr")) {
        if (argc > 1) {
            vfs_node_t *in = vfs_find(cmd[1]);
            if (!in) {
                in = vfs_creat(vfs_getRoot(),cmd[1],0);
            }
            vfs_write(in,0,strlen(cmd[2]),cmd[2]);
        }
    } else if (strcmp(cmd[0],"poweroff")) {
        syscall(15,0,0,0,0,0);
    } else  if (strcmp(cmd[0],"find")) {
        if (argc > 1) {
            vfs_node_t *node = vfs_find(cmd[1]);
            if (!node) {
                printf("find: file not found\n");
            } else {
                printf("File found: %s\n",node->name);
            }
        }
    } else if (strcmp(cmd[0],"test")) {
        // DO PAGE FAULT
        int *p = (int *)0xfffffff;
        *p = 2;
    } else if (strcmp(cmd[0],"fdump")) {
        if (argc > 1) {
            vfs_node_t *file = vfs_finddir(vfs_getRoot(),cmd[1]);
            if (!file) {
                printf("fdump: no such file or directory\n");
                return;
            }
            printf("File name: %s, type: %d, size: %d\n",file->name,file->flags,file->size);
        }
    } else if (strcmp(cmd[0],"waitpid")) {
        if (argc > 1) {
            // DEBUG: watpid test
            int pid = atoi(cmd[1]);
            process_waitPid(pid);
        } else {
            printf("waitpid [pid]\n");
        }
#ifdef DEBUG
    } else if (strcmp(cmd[0],"lockidle")) {
        printf("ATTEMPT TO USE UNDEFINED FUNCTION!\n");
        PANIC("See.");
    } else if (strcmp(cmd[0],"unlockidle")) {
        process_unblock(0);
    } else if (strcmp(cmd[0],"dump")) {
        if (argc == 0) {
            process_dump(process_getProcess(process_getCurrentPID()));
        } else {
            int pid = atoi(cmd[1]);
            process_dump(process_getProcess(pid));
        }
#endif
    } else if (strcmp(cmd[0],"rme")) {
        struct process *child = NULL;
        if ((child = process_create((int)kshell_main,false,"kk",0,NULL)) != NULL) {
            syscall(22,child->pid,0,0,0,0);
        }
    } else if (strcmp(cmd[0],"ps")) {
        process_dumpAll();
    } else if (strcmp(cmd[0],"kill")) {
        if (argc > 1) {
            // kill
            process_kill(atoi(cmd[1]));
        }
    } else if (strcmp(cmd[0],"pid")) {
        printf("%d\n",process_getCurrentPID());
    }  else if (strcmp(cmd[0],"waittime")) {
        int how = atoi(cmd[1]);
        process_wait(process_getCurrentPID(),how);
        struct process *me = process_getProcess(process_getCurrentPID());
        while(me->state == PROCESS_WAITING) {}
    } else if (strcmp(cmd[0],"syms")) {
	symbols_print();
    } else {
        printf("Unknown command: \"%s\"\n",cmd[0]);
    }
}
