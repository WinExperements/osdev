#ifndef INTERRUPTS_H
#define INTERRUPTS_H
#include "typedefs.h"
#include <x86/idt.h>
void interrupts_init();
void interrupts_addHandler(uint8_t,isr_t handler);
void interrupts_block();
void interrupts_unblock();
void interrupts_disable(uint8_t);
void interrupts_enable(uint8_t);
#endif
