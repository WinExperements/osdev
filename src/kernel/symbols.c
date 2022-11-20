#include<symbols.h>
#include<typedefs.h>
#include<elf.h>
#include<terminal.h>
#include<mstring.h>
Elf32_Shdr *symtab;
Elf32_Shdr *strtab;
Elf32_Shdr *symbols_findSection(multiboot_info_t *inf,char *name) {
	struct multiboot_elf_section_header_table t = inf->u.elf_sec;
        uint32_t addr = t.addr;
        uint32_t num = t.num;
        uint32_t shndx = t.shndx;
        Elf32_Shdr *s = (Elf32_Shdr *)addr;
        uint32_t str = s[shndx].sh_addr;
        for (int i = 0; i < num; i++) {
                uint32_t sh_name = s[i].sh_name;
                if (sh_name != 0) {
                        char *s_name = (char *)(str + sh_name);
                        if (strcmp(name,s_name)) {
                                return s + i;
                        }
                }
        }
        return NULL;
}
void symbols_init(multiboot_info_t *inf) {
	struct multiboot_elf_section_header_table t = inf->u.elf_sec;
	printf("num -> %d, size -> %d, addr -> %x, shndx -> %d\n",t.num,t.size,t.addr,t.shndx);
	symtab = symbols_findSection(inf,".symtab");
	strtab = symbols_findSection(inf,".strtab");
	if (symtab == NULL || strtab == NULL) {
		printf("No symbol/string table given by multiboot, abort.\n");
		return;
	} else if (symtab->sh_type != SHT_SYMTAB || strtab->sh_type != SHT_STRTAB) {
		printf("Not valid symtab/strtab!\n");
	}
	printf("%s: symtab: %x, strtab: %x\n",__func__,symtab,strtab);
}
char *symbols_findName(int value) {
	Elf32_Sym *symbol = NULL;
	uint32_t symbol_value = 0;
	char *strtab_addr = (char *)strtab->sh_addr;
        Elf32_Sym *symbols = (Elf32_Sym *)symtab->sh_addr;
        for (int i = 0; i < symtab->sh_size / sizeof(Elf32_Sym); i++) {
                Elf32_Sym *can = symbols + i;
		if (can->st_value > symbol_value && can->st_value <= value) {
                	uint32_t string_index = can->st_name;
                	char *name = strtab_addr + string_index;
                	return name;
		}
        }
	return NULL;
}
int symbols_findValue(char *f_name) {
	char *strtab_addr = (char *)strtab->sh_addr;
        Elf32_Sym *symbols = (Elf32_Sym *)symtab->sh_addr;
        for (int i = 0; i < symtab->sh_size / sizeof(Elf32_Sym *); i++) {
                Elf32_Sym *can = symbols + i;
                uint32_t string_index = can->st_name;
                char *name = strtab_addr + string_index;
                //printf("strtab: %x, index: %d\n",strtab_addr,string_index);
		if (strcmp(f_name,name)) {
			return can->st_value;
		}
        }
	return 0;
}
void symbols_print() {
	char *strtab_addr = (char *)strtab->sh_addr;
        Elf32_Sym *symbols = (Elf32_Sym *)symtab->sh_addr;
        for (int i = 0; i < symtab->sh_size / sizeof(Elf32_Sym); i++) {
                Elf32_Sym *can = symbols + i;
                uint32_t string_index = can->st_name;
                char *name = strtab_addr + string_index;
                printf("%s\n",name);
        }
}
