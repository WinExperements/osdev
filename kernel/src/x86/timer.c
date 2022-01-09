#include "timer.h"
#include "io.h"
#include "interrupt.h"
#include "arch/x86/isr.h"
#include "kernel.h"
void timer_init(u8 freq) {
    u32 divisor = 1193180 / freq;
    io_writePort(0x43,0x36);
    u8 low = (u8)(divisor & 0xFF);
    u8 hight = (u8)((divisor >> 8) & 0xFF);
    io_writePort(0x40,low);
    io_writePort(0x40, hight);
 }