default: all
OBJECTS = stdio.o stdlib.o start.o message.o syscall.o string.o unistd.o dirent.o wait.o sys/mman.o
%.o: %.c
	gcc -m32 -ffreestanding -fPIC -g -nostartfiles -nodefaultlibs -Wno-builtin-declaration-mismatch -I. -c -o $@ $<
%.o: %.asm
	nasm -f elf32 -o $@ $<
all: $(OBJECTS)
	ar rc libcc.a $(OBJECTS)
clean:
	rm *.o
