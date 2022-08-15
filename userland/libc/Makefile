default: all
OBJECTS = stdio.o stdlib.o start.o message.o
%.o: %.c
	gcc -m32 -nostartfiles -nodefaultlibs -c -o $@ $<
%.o: %.asm
	nasm -f elf32 $@ $<
all: $(OBJECTS)
	ar rcs libcc.a $(OBJECTS)
clean:
	rm *.o
