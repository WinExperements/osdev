/* Module loading */
#ifndef MODULE_H
#define MODULE_H
#include <elf.h>
typedef struct module {
    char *name;
    int ref_count;
    struct module_segment *seg;
    Elf32_Sym *symtab;
    void (*init)(struct module *);
    void (*de_init)(struct module *);
} module_t;
typedef struct module_segment
{
    struct module_segment *next;
    void *addr;
    uint32_t size;
    unsigned section;
} module_segment_t;

module_t *load_module(void *address);
#endif
