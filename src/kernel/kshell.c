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
void parseCommand(int argc,char *cmd[]);
multiboot_info_t *info;
vfs_node_t *kshell_tty;
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
    syscall(9,(int)kshell_tty,0,100,(int)buff,0);
}
void kshell_main() {
    // prepare some modules
    printf("Modules: %d\n",info->mods_count);
    vfs_node_t *dev = vfs_finddir(vfs_getRoot(),"dev");
    if (!dev) {
        printf("kshell: no /dev found, execution terminated\n");
        return;
    } else if ((dev->flags & 0x7) != VFS_DIRECTORY) {
        printf("kshell: /dev not a directory, execution terminated\n");
    }
    for (int i = 0; i < info->mods_count; i++) {
        multiboot_module_t *mod = (multiboot_module_t *)info->mods_addr+i;
        if (mod->cmdline != 0) {
            vfs_node_t *in = vfs_creat(dev,(char *)mod->cmdline,0);
            vfs_write(in,0,mod->mod_end-mod->mod_start,(void *)mod->mod_start);
        }
    }
    printf("Kernel debugger shell\n");
    bool exit = false;
    char buff[100];
    char *argv[100];
    while(!exit) {
        int argc = 0;
        printf("> ");
        getInput(buff,100);
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
            vfs_node_t *in = vfs_getRoot();
            if (argc > 1) {
                in = vfs_finddir(in,cmd[1]);
                if (!in){
                    printf("ls: %s: no such file or directory\n");
                }
            }
            struct dirent *d;
            int i = 0;
            while((d = vfs_readdir(in,i)) != 0) {
                printf("%s\n",d->name);
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
    } else if (strcmp(cmd[0],"cat")) {
        if (argc > 1) {
            if (!vfs_getRoot()) {
                printf("cat: no mount point to /\n");
            } else {
                vfs_node_t *n = vfs_finddir(vfs_getRoot(),cmd[1]);
                if (!n) {
                    printf("cat: %s: no such file or directory\n",cmd[1]);
                } else if ((n->flags & 0x7) == VFS_DIRECTORY) {
                    printf("cat: %s: is a directory\n",cmd[1]);
                } else {
                    if (n->size != 0) {
                        int pages = (n->size/4096)+1;
                        char *buf = pmml_allocPages(pages,true);
                        vfs_read(n,0,n->size,buf);
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
                vfs_node_t *n = vfs_finddir(vfs_getRoot(),cmd[1]);
                if (!n) {
                    printf("exec: %s: no such file or directory\n",cmd[1]);
                } else if ((n->flags & 0x7) == VFS_DIRECTORY) {
                    printf("exec: %s: is a directory\n",cmd[1]);
                } else {
                    if (n->size != 0) {
                        int pages = (n->size/4096)+1;
                        void *buf = pmml_allocPages(pages,true);
                        vfs_read(n,0,n->size,buf);
                        syscall(13,(int)buf,(int)n->name,0,0,0);
                        //process_waitPid(-1);
                        pmml_freePages(buf,pages);
                    } else {
                        printf("exec: file zero\n");
                    }
                }
            }
        }
    } else if (strcmp(cmd[0],"execm")){
        multiboot_module_t *mod = (multiboot_module_t *)info->mods_addr;
        syscall(13,mod->mod_start,mod->cmdline,0,0,0);
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
                vfs_changeDir(prev);
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
            prev = vfs_getRoot();
            vfs_changeDir(to);
        } else {
            printf("%s\n",vfs_getRoot()->name);
        }
    } else if (strcmp(cmd[0],"touch")) {
        if (argc > 1) {
            vfs_creat(vfs_getRoot(),cmd[1],0);
        }
    } else if (strcmp(cmd[0],"wr")) {
        if (argc > 1) {
            vfs_node_t *in = vfs_finddir(vfs_getRoot(),cmd[1]);
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
        char *msg = "Pe";
        syscall(1,(int)msg,0,0,0,0);
    } else if (strcmp(cmd[0],"fdump")) {
        if (argc > 1) {
            vfs_node_t *file = vfs_finddir(vfs_getRoot(),cmd[1]);
            if (!file) {
                printf("fdump: no such file or directory\n");
                return;
            }
            printf("File name: %s, type: %d, size: %d\n",file->name,file->flags,file->size);
        }
    } else {
        printf("Unknown command: \"%s\"\n",cmd[0]);
    }
}
