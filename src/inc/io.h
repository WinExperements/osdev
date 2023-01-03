#ifndef IO_H
#define IO_H
#include "typedefs.h"
uint8_t io_readPort(uint32_t port);
void io_writePort(uint32_t port,int data);
//Only works on x86, default throws unsupported architecture exception
void io_writePortW(uint32_t port,uint16_t data);
uint16_t io_readPortW(uint32_t port);
// X86 specific
void x86_insw(uint16_t port, unsigned char *data, unsigned long size);
#endif
