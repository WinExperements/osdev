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
	&sys_poweroff
};
registers_t *syscall_handler(registers_t *regs) {
	if (regs->eax > 15) {
		printf("No such syscall number: %d\n",regs->eax);
		process_kill(process_getCurrentPID());
		process_yield();
	} else {
		arch_enableInterrupts();
		int (*handler)(int,int,int,int,int) = (int (*)(int,int,int,int,int))syscalls[regs->eax];
		regs->eax = handler(regs->edx,regs->ecx,regs->ebx,regs->edi,regs->esi);
	}
	return regs;
}
int sys_print(int p1,int p2,int p3,int p4,int p5) {
	printf((char *)p1);
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
	vfs_node_t *node = vfs_finddir(vfs_getRoot(),(char *)p1);
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
	pmml_freePages((int *)p1,p2);
	return 0;
}
int sys_exec(int p1,int p2,int p3,int p4,int p5) {
	if(elf_load_file((int *)p1,true,(char *)p2)) {
		process_waitPid(1);
	}
	return 0;
}
int sys_reboot(int p1,int p2,int p3,int p4,int p5) {
	printf("SYSTEM GOING REBOOT NOW!\n");
	printf("Press enter to confirm reboot. Or press \"n\" to cancel: ");
	if (keyboard_get() == 'n') {
		printf("\nCancelled\n");
		return 0;
	}
	arch_reset();
	return 0; // should not happen
}
int sys_poweroff(int p1,int p2,int p3,int p4,int p5) {
	arch_poweroff();
	return 0;
}