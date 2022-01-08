#ifndef IO_H
#define IO_H
#include "typedefs.h"
u8 io_readPort(u32 port);
void io_writePort(u32 port,int data);
//Only works on x86, default throws unsupported architecture exception
u16 io_writePortW(u32 port);
#endif