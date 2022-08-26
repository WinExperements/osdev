#include<typedefs.h>
#include<elf.h>
#include<mm/pmm.h>
#include<module.h>
#include<terminal.h>
#include<mstring.h>
#include<mm.h>
bool module_resolve_name(module_t *mod,Elf32_Ehdr *);
bool module_resolve_dep(module_t *,Elf32_Ehdr *);
bool module_load_seg(module_t *mod,Elf32_Ehdr *);
bool module_resolve_symbols(module_t *mod,Elf32_Ehdr *);
bool module_reloc_symbols(module_t *,Elf32_Ehdr *);
module_t *load_module(void *address) {
    Elf32_Ehdr *header = (Elf32_Ehdr *)address;
    module_t *mod = pmml_alloc(true);
    if (!mod) {
        printf("Seems to your allocator broken\n");
        return NULL;
    }
    if (!elf_check_file(header)) {
        return NULL;
    }
    if (!module_resolve_name(mod,header) || !module_resolve_dep(mod,header)
        || !module_load_seg(mod,header) || !module_resolve_symbols(mod,header)
        || !module_reloc_symbols(mod,header)) {
            return NULL;
        }
    return mod;
}
bool module_resolve_name(module_t *mod,Elf32_Ehdr *header) {
    Elf32_Shdr *s;
    unsigned i;
    const char *str;
    s = (Elf32_Shdr *)((char *)header + header->e_shoff + header->e_shstrndx * header->e_shentsize);
    str = (char *)header + s->sh_offset;
    for (i = 0,s = (Elf32_Shdr *)((char *)header + header->e_shoff); i < header->e_shnum;
        i++,s = (Elf32_Shdr *)((char *)s + header->e_shentsize)) {
            if (strcmp((char *)str + s->sh_name,".modname")) {
                mod->name = strdup((char *)header + s->sh_offset);
                if (!mod->name) {
                    printf("Failed to set module name\n");
                    return false;
                }
                break;
            }
        }
    if (i == header->e_shnum) {
        printf("No module name\n");
        return false;
    }
    return true;
}
bool module_resolve_dep(module_t *mod,Elf32_Ehdr *e) {
    Elf32_Shdr *s = NULL;
    const char *str;
    unsigned i;
    s = (Elf32_Shdr *)((char *) e + e->e_shoff + e->e_shstrndx * e->e_shentsize);
    str = (char *)e + s->sh_offset;
    for (i = 0,s = (Elf32_Shdr *)((char *)e + e->e_shoff); i < e->e_shnum;
        i++,s = (Elf32_Shdr *)((char *)s + e->e_shentsize)) {
            if (strcmp((char *)str+ s->sh_name,".moddeps")) {
                printf("warrning: modules deps didn't supported\n");
                return true;
            }
    }
    return true;
}
bool module_load_seg(module_t *mod,Elf32_Ehdr *e) {
    unsigned i;
    Elf32_Shdr *s;
    for (i = 0,s = (Elf32_Shdr *)((char *)e + e->e_shoff);
        i < e->e_shnum;
        i++,s = (Elf32_Shdr *)((char *)s+e->e_shentsize)) {
            if (s->sh_flags & (1<<1)) {
                module_segment_t *seg;
                seg = pmml_alloc(true);
                if (!seg) return false;
                if (s->sh_size) {
                    void *address = pmml_allocPages((s->sh_size/4096)+1,false) + s->sh_addralign;
                    if (!address) {
                        pmml_free(seg);
                        return false;
                    }
                    switch (s->sh_type)
                    {
                        case 1: 
                        memcpy(address,(void *)e + s->sh_offset,s->sh_size);
                        break;
                        case 8:
                        memset(address,0,s->sh_size);
                    }
                    seg->addr = address;
                } else {
                    seg->addr = 0;
                }
                seg->size = s->sh_size;
                seg->section = i;
                seg->next = mod->seg;
                mod->seg = seg;
            }
        }
        return true;
}
bool module_resolve_symbols(module_t *mod,Elf32_Ehdr *e) {
    return false;
}
bool module_reloc_symbols(module_t *mod,Elf32_Ehdr *e) {
    return false;
}