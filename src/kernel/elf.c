#include<elf.h>
#include<mm.h>
#include<mm/pmm.h>
#include<terminal.h>
#include<mstring.h>
#include<x86/vmm_x86.h>
#include<module.h>
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
bool elf_load_file(void *file,bool isUser,char *name) {
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
	case 1: {
        module_t *module = load_module(file);
        if (!module || !module->init) {
            printf("Failed to load module\n");
            return false;
        }
        module->init(module);
        return true;
    }
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
    struct process *p = process_create(header->e_entry,isUser,name);
    p->dir = (uint32_t)dir;
    p->pages = pages_s;
    return true;
}
