#include "io.h"
void io_writePort(u32 port,int data) {
	asm("outb %b0, %w1": :"a"(data), "Nd"(port));
}
u8 io_readPort(u32 port) {
	u8 result;
      asm("inb %w1, %b0": "=a"(result):"Nd"(port));
	return result;
}
u16 io_readPort16(u32 port) {
	u16 result;
      asm("inw %w1, %w0": "=a"(result):"Nd"(port));
	return result;
}