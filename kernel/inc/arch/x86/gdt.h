#ifndef GDT_H
#define GDT_H
#include "typedefs.h"
struct gdt_entry_struct
{
  u16 limit_low;           // The lower 16 bits of the limit.
  u16 base_low;            // The lower 16 bits of the base.
  u8  base_middle;         // The next 8 bits of the base.
  u8  access;              // Access flags, determine what ring this segment can be used in.
  u8  granularity;
  u8  base_high;           // The last 8 bits of the base.
} __attribute__((packed));
typedef struct gdt_entry_struct gdt_entry_t;

// @ Desc : Define the pointer which is used to tell the processor where to find our GDT
struct gdt_ptr_struct
{
  u16 limit;               // The upper 16 bits of all selector limits.
  u32 base;                // The address of the first gdt_entry_t struct.
}
__attribute__((packed));
typedef struct gdt_ptr_struct gdt_ptr_t;
void gdt_init();
void gdt_set_gate(s32,u32,u32,u8,u8);
extern void gdt_flush(int);
#endif
