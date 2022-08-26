default: all
OBJECTS = stdio.o stdlib.o start.o message.o syscall.o
%.o: %.c
	i686-elf-gcc -m32 -nostartfiles -nodefaultlibs -c -o $@ $<
%.o: %.asm
	nasm -f elf32 $@ $<
all: $(OBJECTS)
	ar rcs libcc.a $(OBJECTS)
	ld -melf_i386 -fpic -shared $(OBJECTS) -o g.so
clean:
	del *.o
