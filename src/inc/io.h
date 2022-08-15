#ifndef IO_H
#define IO_H
#include "typedefs.h"
uint8_t io_readPort(uint32_t port);
void io_writePort(uint32_t port,int data);
//Only works on x86, default throws unsupported architecture exception
uint16_t io_writePortW(uint32_t port);
#endif
