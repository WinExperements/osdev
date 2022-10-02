#include<elf.h>
#include<mm.h>
#include<mm/pmm.h>
#include<terminal.h>
#include<mstring.h>
#include<x86/vmm_x86.h>
#include<module.h>
#include<arch.h>
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
bool elf_load_file(void *file,bool isUser,char *name,int argc,char **argv,char **envp) {
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
    int *dir = NULL;
    int pages_s = 0;
    size_t elf_base = (size_t)header;
    int page_start = 0;
    for (pe = 0; pe < header->e_phnum; pe++) {
        Elf32_Phdr *p_entry = (void *)(header->e_phoff + elf_base + pe * header->e_phentsize);
        if (p_entry->p_type == 1) {
            if (p_entry->p_memsz == 0) {
                continue; // skip
            }
            if (dir == 0) {
                dir = vmm_createDirectory();
                pages_s+=2;
            }
             if (page_start == 0) {
                page_start = (int)dir;
            }
            int pages = (p_entry->p_memsz/4096)+1;
            pages_s+=pages;
            void *section = (void *)(elf_base+p_entry->p_offset);
            void *to = pmml_allocPages(pages,true);
            int vaddr = p_entry->p_vaddr;
            if (page_start == 0) {
                page_start = vaddr;
            }
            int *table = pmml_alloc(true);
            pages_s+=2;
            memcpy(to,section,p_entry->p_memsz);
            for (int i = 0; i < pages; i++) {
                int addr = ((int)to)+(i*4096);
                vmm_map(table,addr,vaddr);
                vaddr+=4096;
            }
            dir[PAGE_DIRECTORY_INDEX(vaddr)] = ((unsigned int)table) | 7;
        }
    }
    struct process *p = process_create(header->e_entry,isUser,name);
    if (!p) {
        pmml_freePages((void *)page_start,pages_s);
    }
    p->dir = (uint32_t)dir;
    p->pages = pages_s;
    p->page_start = page_start;
    if (argc == 0 || argv == 0) {
        char **argva = pmml_alloc(true); // arguments
        argva[0] = "<unknown>";
        argva[1] = "init";
        if (envp == NULL) {
             envp = pmml_alloc(true);
            envp[0] = "PATH=/bin";
        }
        arch_copy_process_args(p,2,argva,envp);
    } else {
        arch_copy_process_args(p,argc,argv,envp);
    }
    return true;
}
