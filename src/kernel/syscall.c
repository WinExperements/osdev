#include <syscall.h>
#include <interrupts.h>
#include <terminal.h>
#include <process.h>
#include<dev.h>
#include <ipc/message.h>
#include <mm/pmm.h>
#include <mstring.h>
#include<io.h>
#include<vfs.h>
#include<elf.h>
#include<keyboard.h>
#include<arch.h>
#include<serial.h>
#include<kshell.h>
static void *syscalls[] = {
	0,
	&sys_print,
	&sys_exit,
	&sys_kill,
	&sys_getpid,
	&sys_sendto,
	&sys_recev,
	&sys_open,
	&sys_close,
	&sys_read,
	&sys_write,
	&sys_alloc,
	&sys_free,
	&sys_exec,
	&sys_reboot,
	&sys_poweroff,
	&sys_pwd,
	&sys_chdir,
	&sys_opendir,
	&sys_closedir,
	&sys_readdir,
	&sys_exec_shell,
    &sys_waitpid,
    &sys_getppid,
    &sys_sysinfo,
    &sys_getuid,
    &sys_setuid
};
registers_t *syscall_handler(registers_t *regs) {
	if (regs->eax > 26) {
		printf("No such syscall number: %d\n",regs->eax);
	} else {
        arch_enableInterrupts();
		int (*handler)(int,int,int,int,int) = (int (*)(int,int,int,int,int))syscalls[regs->eax];
		regs->eax = handler(regs->edx,regs->ecx,regs->ebx,regs->edi,regs->esi);
	}
	return regs;
}
int sys_print(int p1,int p2,int p3,int p4,int p5) {
	char *msg = (char *)p1;
	if (strcmp(msg,"\033[2J")) {
		terminal_clear();
	} else {
		printf(msg);
	}
	return 0;
}
int sys_exit(int p1,int p2,int p3,int p4,int p5) {
	process_exit(process_getCurrentPID(),0);
	process_yield();
	return 0;
}
int sys_kill(int p1,int p2,int p3,int p4,int p5) {
	process_kill(p1);
	return 0;
}
int sys_getpid(int p1,int p2,int p3,int p4,int p5) {
	return process_getCurrentPID();
}
int sys_sendto(int p1,int p2,int p3,int p4,int p5) {
	char *m = (char *)vmm_translate((int *)process_getProcess(process_getCurrentPID())->dir,p2);
	return msg_send(p1,0,m);
}
int sys_recev(int p1,int p2,int p3,int p4,int p5) {
	msg_t message = msg_receive(p1);
		if (message.len == 0) {
		printf("No incomming messages for %d\n",p1);
		return 0;
	} else {
		int addr = &message;
		return addr;
	}
}
int sys_open(int p1,int p2,int p3,int p4,int p5) {
	int flags = p2;
	vfs_node_t *node = vfs_find((char *)p1);
	if (!node && flags == 7) {
		node = vfs_creat(vfs_getRoot(),(char *)p1,0);
	}
	return (int)node;
}
int sys_close(int p1,int p2,int p3,int p4,int p5) {
	vfs_close((vfs_node_t *)p1);
	return 0;
}
int sys_read(int p1,int p2,int p3,int p4,int p5) {
	vfs_read((vfs_node_t *)p1,p2,p3,(int *)p4);
	return 0;
}
int sys_write(int p1,int p2,int p3,int p4,int p5) {
	vfs_write((vfs_node_t *)p1,p2,p3,(int *)p4);
	return 0;
}
int sys_alloc(int p1,int p2,int p3,int p4,int p5) {
	return (int)pmml_allocPages(p1,true);
}
int sys_free(int p1,int p2,int p3,int p4,int p5) {
	if (p1 == 0) {
		printf("Freeing address 0 doesn't allowed!\n");
		process_kill(process_getCurrentPID());
	}
	pmml_freePages((int *)p1,p2);
	return 0;
}
int sys_exec(int p1,int p2,int p3,int p4,int p5) {
	char *path = (char *)p1;
	vfs_node_t *node = vfs_find(path);
	if (!node) return -1;
	if ((node->flags & 0x7) == VFS_DIRECTORY) {
		printf("%s: directory\n",p1);
		return -2;
	}
	int pages = (node->size/4096)+1;
	void *addr = pmml_allocPages(pages,true);
	vfs_read(node,0,node->size,addr);
	if (!elf_load_file(addr,true,node->name,p2,(char **)p3,(char **)p4)) {
        	printf("exec: loading elf failed!\n");
            pmml_freePages(addr,pages);
        	arch_enableInterrupts();
		return -2;
	}
	pmml_freePages(addr,pages);
   	// process_dump(process_getProcess(process_getProcesses()-1));
	return process_getNextPID()-1;
}
int sys_reboot(int p1,int p2,int p3,int p4,int p5) {
	arch_reset();
	return 0; // should not happen
}
int sys_poweroff(int p1,int p2,int p3,int p4,int p5) {
	arch_poweroff();
	return 0;
}
// Change working directory for current process(caller)
int sys_chdir(int to,int p2,int p3,int p4,int p5) {
	// find the caller
	struct process *caller = process_getProcess(process_getCurrentPID());
	if (!caller) {
		// very strange, should never happen, panic
		PANIC("Cannot get caller!\n");
	}
	vfs_node_t *node = vfs_find((char *)to);
	if (!node) {
		printf("chdir: %s: no such file or directory\n",(char *)to);
		return -1;
	}
	caller->workDir = node;
	return 0;
}
int sys_pwd(int p1,int p2,int p3,int p4,int p5) {
	vfs_node_path(process_getProcess(process_getCurrentPID())->workDir,(char *)p1,p2);
	return 0;
}
int sys_opendir(int p1,int p2,int p3,int p4,int p5) {
	// Open dir, is a just like sys_open
	return sys_open((int)p1,0,0,0,0);
}
int sys_closedir(int p1,int p2,int p3,int p4,int p5) {
	// Just like sys_close
	return 0;
}
int sys_readdir(int p1,int p2,int p3,int p4,int p5) {
	vfs_node_t *node = (vfs_node_t *)p1;
	if (node) {
		return (int)vfs_readdir(node,p2);
	}
	return 0;
}
/* Mount syscall not shell execution !! */
int sys_exec_shell(int p1,int p2,int p3,int p4,int p5) {
    	// allocate and copy parameters from user
	char *dev = copy_from_user((void *)p1);
	char *_mount_dir = copy_from_user((void *)p2);
	char *_fs = copy_from_user((void *)p3);
	vfs_node_t *dev_path = vfs_find(dev);
    	if (dev_path == NULL) {
		pmml_free(dev);
                pmml_free(_mount_dir);
                pmml_free(_fs);
		return -2;
	}
    	vfs_node_t *mount_dir = vfs_find(_mount_dir);
    	if (mount_dir == NULL) {
		pmml_free(dev);
                pmml_free(_mount_dir);
                pmml_free(_fs);
		return -3;
	}
    	vfs_fs_t *fs = vfs_findFS(_fs);
    	if (fs == NULL) {
		pmml_free(dev);
		pmml_free(_mount_dir);
		pmml_free(_fs);
		return -4;
	}
    	return 0;
}
int sys_waitpid(int p1,int p2,int p3,int p4,int p5) {
    if (p1 > 0) {
        //check if given process has spawned by current caller
        struct process *child = process_getProcess(p1);
        int parent = process_getCurrentPID();
        if (child != NULL && child->parent == parent) {
            process_waitPid(parent);
            struct process *p = process_getProcess(parent);
            while(p->state == PROCESS_WAITPID) {}
        }
    }
    return 0;
}
int sys_getppid(int p1,int p2,int p3,int p4,int p5) {
    struct process *parent = process_getProcess(process_getProcess(process_getCurrentPID())->parent);
    if (parent != NULL) {
        return parent->pid;
    }
    return 0;
}
int sys_sysinfo(int p1,int p2,int p3,int p4,int p5) {
    int all = pmml_getMaxBlocks()*4096/1024;
    int free = pmml_getFreeBlocks()*4096/1024;
    int used = all-free;
    printf("HelinOS compiled with GCC %s. Kernel compiled at %s:%s\n",__VERSION__,__DATE__,__TIME__);
    printf("Memory detected: %dKB, used: %dKB, free: %dKB %d pages\n",all,used,free,pmml_getFreeBlocks());
    return 0;
}
void *copy_from_user(void *from) {
	if (!from) return NULL;
	void *ret = pmml_alloc(true);
	memcpy(ret,from,strlen((char *)from));
	return ret;
}
int sys_getuid(int p1,int p2,int p3,int p4,int p5) {
    // Return UID from running task
    return process_getProcess(process_getCurrentPID())->uid;
}
int sys_setuid(int p1,int p2,int p3,int p4,int p5) {
    process_getProcess(process_getCurrentPID())->uid = p1;
    return 0;
}
