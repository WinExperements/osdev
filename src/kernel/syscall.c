#include <syscall.h>
#include <interrupts.h>
#include <terminal.h>
#include <process.h>
#include<dev.h>
#include <ipc/message.h>
#include <mm/pmm.h>
#include <mstring.h>
#include<io.h>
registers_t *syscall_handler(registers_t *regs) {
	if (regs->eax > 9) {
		printf("No such syscall number: %d\n",regs->eax);
	} else {
		if (regs->eax == 1) {
			printf((char *)regs->ebx);
		} else if (regs->eax == 2) { // process exit
			process_exit(process_getCurrentPID(),0);
			asm volatile("int $32"); // re-schedule
		} else if (regs->eax == 3) {
			process_kill(regs->ebx);
		} else if (regs->eax == 4) {
			regs->eax = process_getCurrentPID();
	 	} else if (regs->eax == 5)  {
			// Send message to process
			// Translate virtual address to physical, then call msg_send
			char *m = (char *)vmm_translate((int *)process_getProcess(process_getCurrentPID())->dir,regs->ecx);
			regs->eax = msg_send(regs->ebx,0,m);
		} else if (regs->eax == 6) {
			msg_t message = msg_receive(regs->ebx);
			if (message.len == 0) {
				printf("No incomming messages for %d\n",regs->ebx);
				regs->eax = 0;
				return regs;
			} else {
				regs->eax = (int)&message;
			}
		} else if (regs->eax == 8) {
			io_writePort(regs->ebx,regs->ecx);
		} else if (regs->eax == 9) {
			regs->eax = io_readPort(regs->ebx);
		}
	}
	return regs;
}
