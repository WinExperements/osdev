#ifndef INTERRUPT_H
#define INTERRUPT_H
#include "typedefs.h"
void intr_init();
void intr_add(u8,void (*handler)());
#endif