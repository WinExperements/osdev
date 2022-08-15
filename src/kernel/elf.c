#include<elf.h>
#include<mm.h>
#include<mm/pmm.h>
#include<terminal.h>
#include<mstring.h>
#include<x86/vmm_x86.h>
int elf_do_relloc(Elf32_Ehdr *hdr,Elf32_Rel *reltab,Elf32_Shdr *section);
void *elf_lookup_symbol(const char *name);
int DO_386_32(int symval,int ref) {return 0;}
int DO_386_PC32(int symval,int ref,int refaddr) {return 0;}
bool elf_check_file(Elf32_Ehdr *hdr) {
    if (!hdr) {
        printf("elf: given header are null\n");
        return false;
    }
    if(hdr->e_ident[EI_MAG0] != ELFMAG0) {
		printf("ELF Header EI_MAG0 incorrect.\n");
		return false;
	}
	if(hdr->e_ident[EI_MAG1] != ELFMAG1) {
		printf("ELF Header EI_MAG1 incorrect.\n");
		return false;
	}
	if(hdr->e_ident[EI_MAG2] != ELFMAG2) {
		printf("ELF Header EI_MAG2 incorrect.\n");
		return false;
	}
	if(hdr->e_ident[EI_MAG3] != ELFMAG3) {
		printf("ELF Header EI_MAG3 incorrect.\n");
		return false;
	}
    return true;
}
/* Load elf then automatically create process */
bool elf_load_file(void *file,bool isUser) {
    int pe;
    Elf32_Ehdr *header = (Elf32_Ehdr *)file;
    if (!elf_check_file(header)) {
        printf("%s: not a ELF file, check erros\n",__func__);
        return false;
    }
    // check if object are relocable or executable
    switch(header->e_type) {
	case 2:
	break;
	case 1:
	return elf_load_rel_file(file);
	break;
	default:
	return false;
    }
    if (header->e_machine != 3) {
        printf("elf: trying to execute non i386 executable\n");
    }
    Elf32_Phdr *p_entry = (file+header->e_phoff);
    int *dir = NULL;
    int pages_s = 0;
    for (pe = 0; pe < header->e_phnum; pe++,p_entry++) {
        if (p_entry->p_type == 1) {
            if (p_entry->p_memsz == 0) {
                continue; // skip
            }
            if (dir == 0) {
                dir = vmm_createDirectory();
                pages_s+=3;
            }
            int pages = (p_entry->p_memsz/4096)+1;
            void *section = (void *)(file+p_entry->p_offset);
            void *to = pmml_allocPages(pages,true);
            memcpy(to,section,p_entry->p_memsz);
            for (int i = 0; i < pages; i++) {
                int addr = ((int)to)+i * 4096;
                int *table = pmml_alloc(true);
                if (table == NULL) {
                    printf("%s: cannot init virtual memory for process\n",__func__);
                    return false;
                }
                vmm_map(table,addr,p_entry->p_vaddr);
                dir[PAGE_DIRECTORY_INDEX(p_entry->p_vaddr)] = ((unsigned int)table) | 7;
                pages_s++;
            }
        }
    }
    struct process *p = process_create(header->e_entry,isUser,"elf");
    p->dir = (uint32_t)dir;
    p->pages = pages_s;
    return true;
}
bool elf_load_rel_file(void *fileAddress) {
    Elf32_Ehdr *header = (Elf32_Ehdr *)fileAddress;
    Elf32_Shdr *shdr = (Elf32_Shdr *)((int)header + header->e_shoff);
    unsigned int i,idx;
    for (i = 0; i < header->e_shnum; i++) {
	Elf32_Shdr *section = &shdr[i];
	if (section->sh_type == SHT_REL) {
		for (idx = 0; idx < section->sh_size / section->sh_entsize; idx++) {
			Elf32_Rel *reltab = &((Elf32_Rel *)((int)header + section->sh_offset))[idx];
			if (elf_do_relloc(header,reltab,section) == -1) {
				printf("%s: Failed to realloc symbol\n",__func__);
				return false;
			}
		}
	}
	}
    return false;
}
int elf_get_symval(Elf32_Ehdr *hdr,int table,uint32_t idx) {
    if (table == 0|| idx == 0) return 0;
    Elf32_Shdr *symtab = elf_section(hdr,idx);
    uint32_t symtab_entites = symtab->sh_size / symtab->sh_entsize;
    if (idx >= symtab_entites) {
        printf("%s: symbol out of range\n",__func__);
        return -1;
    }
    int symaddr = (int)hdr + symtab->sh_offset;
    Elf32_Sym *symbol = &((Elf32_Sym *)symaddr)[idx];
    if (symbol->st_shndx == SHN_UNDEF) {
        Elf32_Shdr *strtab = elf_section(hdr,symtab->sh_link);
        const char *name = (const char *)hdr + strtab->sh_offset + symbol->st_name;
        printf("%s: name: %s\n",__func__,name);
        void *target = elf_lookup_symbol(name);
        if (target == NULL) {
            if (ELF32_ST_BIND(symbol->st_info) & STB_WEAK) {
                return 0;
            } else {
                printf("%s: Undefined symbol: %s\n",__func__,name);
                return -1;
            }
        } else {
            return (int)target;
        }
    } else if (symbol->st_shndx == SHN_ABS) {
        return symbol->st_value;
    } else {
        Elf32_Shdr *target = elf_section(hdr,symbol->st_shndx);
        return (int)hdr + symbol->st_value + target->sh_offset;
    }
}
/** Relloc any symbol, very needed for the modules **/
int elf_do_relloc(Elf32_Ehdr *hdr,Elf32_Rel *reltab,Elf32_Shdr *section) {
	Elf32_Shdr *target = elf_section(hdr,section->sh_info);
	int addr = (int)hdr + target->sh_offset;
	int *ref = (int *)(addr + reltab->r_offset);
	int symval = 0;
    if (ELF32_R_SYM(reltab->r_info) != SHN_UNDEF) {
        symval = elf_get_symval(hdr,section->sh_link,ELF32_R_SYM(reltab->r_info));
        if (symval == -1) return -1;
    }
    switch ((ELF32_R_TYPE(reltab->r_info)))
    {
    case R_386_NONE:
        break;
    case R_386_32:
        *ref = DO_386_32(symval,*ref);
        break;
    case R_386_PC32:
        *ref = DO_386_PC32(symval,*ref,(int)ref);
        break;
    
    default:
        printf("Unsupported symbol type: %d\n",ELF32_R_TYPE(reltab->r_info));
        return -1;
    }
	return -1;
}
void *elf_lookup_symbol(const char *name) {
    return NULL;
}
