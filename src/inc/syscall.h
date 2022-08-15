#ifndef SYSCALL_H
#define SYSCALL_H
#include <typedefs.h>
#include <x86/idt.h>
registers_t *syscall_handler(registers_t *regs);
#endif