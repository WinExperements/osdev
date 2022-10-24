#ifndef SYSCALL_H
#define SYSCALL_H
#include <typedefs.h>
#include <x86/idt.h>
// Yeah wierd code
int sys_print(int p1,int p2,int p3,int p4,int p5);
int sys_exit(int p1,int p2,int p3,int p4,int p5);
int sys_kill(int p1,int p2,int p3,int p4,int p5);
int sys_getpid(int p1,int p2,int p3,int p4,int p5);
int sys_sendto(int p1,int p2,int p3,int p4,int p5);
int sys_recev(int p1,int p2,int p3,int p4,int p5);
int sys_open(int p1,int p2,int p3,int p4,int p5);
int sys_close(int p1,int p2,int p3,int p4,int p5);
int sys_read(int p1,int p2,int p3,int p4,int p5);
int sys_write(int p1,int p2,int p3,int p4,int p5);
int sys_alloc(int p1,int p2,int p3,int p4,int p5);
int sys_free(int p1,int p2,int p3,int p4,int p5);
int sys_exec(int p1,int p2,int p3,int p4,int p5);
int sys_reboot(int p1,int p2,int p3,int p4,int p5);
int sys_poweroff(int p1,int p2,int p3,int p4,int p5);
int sys_pwd(int p1,int p2,int p3,int p4,int p5);
int sys_chdir(int p1,int p2,int p3,int p4,int p5);
int sys_opendir(int p1,int p2,int p3,int p4,int p5);
int sys_closedir(int p1,int p2,int p3,int p4,int p5);
int sys_readdir(int p1,int p2,int p3,int p4,int p5);
int sys_exec_shell(int p1,int p2,int p3,int p4,int p5);
int sys_waitpid(int p1,int p2,int p3,int p4,int p5);
int sys_getppid(int p1,int p2,int p3,int p4,int p5);
int sys_sysinfo(int p1,int p2,int p3,int p4,int p5);
int sys_getuid(int p1,int p2,int p3,int p4,int p5);
int sys_setuid(int p1,int p2,int p3,int p4,int p5);
registers_t *syscall_handler(registers_t *regs);
/*
 * Because we can't directly modify user memory,
 * we need to copy it into kernel using allocation and memcpy
*/
void *copy_from_user(void *from);
#endif
