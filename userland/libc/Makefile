default: all
OBJECTS = stdio.o stdlib.o start.o message.o syscall.o string.o unistd.o dirent.o wait.o
%.o: %.c
	gcc -g -m32 -nostartfiles -nodefaultlibs -Wno-builtin-declaration-mismatch -c -o $@ $<
%.o: %.asm
	nasm -f elf32 -o $@ $<
all: $(OBJECTS)
	ar rcs libcc.a $(OBJECTS)
clean:
	rm *.o
