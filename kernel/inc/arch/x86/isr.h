#ifndef ISR_H
#define ISR_H
#include "typedefs.h"
#define IRQ0 32
#define IRQ1 33
#define IRQ2 34
#define IRQ3 35
#define IRQ4 36
#define IRQ5 37
#define IRQ6 38
#define IRQ7 39
#define IRQ8 40
#define IRQ9 41
#define IRQ10 42
#define IRQ11 43
#define IRQ12 44
#define IRQ13 45
#define IRQ14 46
#define IRQ15 47
typedef struct registers
{
  u32 ds;                  // Data segment selector
  u32 edi;
  u32 esi;
  u32 ebp;
  u32 esp;
  u32 ebx;
  u32 edx;
  u32 ecx;
  u32 eax;
  // Interrupt number and error code (if applicable)
  u32 int_no, err_code;
  // Pushed by the processor automatically
  u32 eip;
  u32 cs;
  u32 eflags;
  u32 useresp;
  u32 ss;
} registers_t;
typedef void (*isr_t)(registers_t);
void isr_handler(registers_t);
void irq_handler(registers_t);
#endif
