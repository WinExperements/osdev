#ifndef TERMINAL_H
#define TERMINAL_H
#include "typedefs.h"
void terminal_initialize(struct multiboot_info *info);
void terminal_setcolor(uint32_t colo);
void terminal_setbackground(uint32_t back);
void terminal_write(const char* data, size_t size);
void terminal_writestring(const char* data);
void terminal_writeInt(int u);
void terminal_writeHex(int num);
void printf(char *format,...);
int terminal_getX();
int terminal_getY();
int terminal_getBufferAddress();
void terminal_clearWithColor(uint32_t back);
void terminal_clear();
void printf_syscall(const char *);
void terminal_enableReplay(bool enable);
void terminal_writeXY(char c,uint16_t x,uint16_t y);
void terminal_printCursor(uint16_t x,uint16_t y);
int terminal_getBufferSize();
void tty_init();
#endif
