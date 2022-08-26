#include <x86/pic.h>
#include <io.h>
#define PIC_ICW1 0x11
#define PIC_ICW4_MASTER 0x01
#define PIC_ICW4_SLAVE  0x05
#define PIC_ACK_SPECIFIC 0x60
static uint8_t pic_control[2] = { 0x20, 0xa0 };
static uint8_t pic_data[2] = { 0x21, 0xa1 };
void pic_init(int base0,int base1) {
	io_writePort(PIC_ICW1,pic_control[0]);
	io_writePort(base0,pic_data[0]);
	io_writePort(1 << 2,pic_data[0]);
	io_writePort(PIC_ICW4_MASTER, pic_data[0]);
	io_writePort(~(1 << 2), pic_data[0]);
	io_writePort(PIC_ICW1, pic_control[1]);
	io_writePort(base1, pic_data[1]);
	io_writePort(2, pic_data[1]);
	io_writePort(PIC_ICW4_SLAVE, pic_data[1]);
	io_writePort(~0, pic_data[1]);
}
void pic_enable(uint8_t irq) {
	uint8_t mask;
	if(irq < 8) {
		mask = io_readPort(pic_data[0]);
		mask = mask & ~(1 << irq);
		io_writePort(mask, pic_data[0]);
	} else {
		irq -= 8;
		mask = io_readPort(pic_data[1]);
		mask = mask & ~(1 << irq);
		io_writePort(mask, pic_data[1]);
		pic_enable(2);
	}
}
void pic_disable(uint8_t irq) {
	uint8_t mask;
	if(irq < 8) {
		mask = io_readPort(pic_data[0]);
		mask = mask | (1 << irq);
		io_writePort(mask, pic_data[0]);
	} else {
		irq -= 8;
		mask = io_readPort(pic_data[1]);
		mask = mask | (1 << irq);
		io_writePort(mask, pic_data[1]);
	}
}
void pic_acknowledge(uint8_t irq)
{
	if(irq >= 8) {
		io_writePort(PIC_ACK_SPECIFIC + (irq - 8), pic_control[1]);
		io_writePort(PIC_ACK_SPECIFIC + (2), pic_control[0]);
	} else {
		io_writePort(PIC_ACK_SPECIFIC + irq, pic_control[0]);
	}
}