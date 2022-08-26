#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libelf.h>
#include <gelf.h>

void
main(int argc, char **argv)
{
    Elf         *elf;
    Elf_Scn     *scn = NULL;
    GElf_Shdr   shdr;
    Elf_Data    *data;
    int         fd, ii, count;

    elf_version(EV_CURRENT);

    fd = open(argv[1], O_RDONLY);
    elf = elf_begin(fd, ELF_C_READ, NULL);

    while ((scn = elf_nextscn(elf, scn)) != NULL) {
        gelf_getshdr(scn, &shdr);
        if (shdr.sh_type == SHT_SYMTAB) {
            /* found a symbol table, go print it. */
            break;
        }
    }
    FILE *out = fopen("symbols.h","w");
    if (!out) {
	printf("Failed to open output file\n");
	return;
    }
    fputs("/* This file is auto-generated */\n\n",out);
    fputs("#ifndef SYMBOLS_H\n#define SYMBOLS_H",out);
    fputs("\n#include<typedefs.h>\n",out);
    data = elf_getdata(scn, NULL);
    count = shdr.sh_size / shdr.sh_entsize;
    fprintf(out,"int ksymbols_count = %d;\n",count);
    fputs("symbol_t ksymbols[] = {\n   ",out);
    /* print the symbol names */
    for (ii = 0; ii < count; ++ii) {
        GElf_Sym sym;
        gelf_getsym(data, ii, &sym);
        char *name = elf_strptr(elf, shdr.sh_link, sym.st_name);
	    fprintf(out,"{\"%s\",%d},\n   ",name,sym.st_value);
    }
    elf_end(elf);
    close(fd);
    fputs("{0,0}\n};\n",out);
    fputs("#endif",out);
    fclose(out);
}
