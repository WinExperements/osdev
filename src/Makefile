ARCH=x86
CCOPTIONS = -m32 -ggdb -std=gnu99 -ffreestanding -Wall -Wextra -Wunused-parameter -Wint-to-pointer-cast -Wsign-compare -nostdlib -march=i486 -mtune=i486 -fno-stack-protector -DLEGACY_TERMINAL
ASPARAMS =
CCPATH = i686-elf-
OBJECTS = arch\$(ARCH)\boot.o arch\$(ARCH)\terminal.o kernel\kernel.o arch\$(ARCH)\io.o arch\$(ARCH)\serial.o arch\$(ARCH)\arch.o arch\$(ARCH)\poweroff.o arch\$(ARCH)\gdt.o arch\$(ARCH)\gdtfl.o arch\x86\interrupts.o arch\x86\interruptsasm.o mm\mm.o mm\pmm.o arch\$(ARCH)\vmm.o kernel\mstring.o kernel\process.o arch\$(ARCH)\switch.o arch\$(ARCH)\mmu.o kernel\syscall.o kernel\keyboard.o arch\x86\pic.o kernel\elf.o font.o kernel\dev.o ipc\message.o kernel\vfs.o
MODULE_OBJS = module\atapi\atapi.o
%.o: %.c
	$(CCPATH)gcc $(CCOPTIONS) -Iinc -Imodule -c -o $@ $<
%.o: %.s
	$(CCPATH)as --32 -g -o $@ $<
%.o: %.asm
	D:\nasm\nasm -f elf32 -F dwarf -g -o $@ $<
%.o: %.psf
	$(CCPATH)objcopy -O elf32-i386 -B i386 -I binary $< $@
all: $(OBJECTS) $(MODULE_OBJS)
	$(CCPATH)ld -melf_i386 -T arch\$(ARCH)\linker.ld -Map=kernel.map -o kernel.bin $(OBJECTS) $(MODULE_OBJS)
	@echo Compiled
clean:
	del $(OBJECTS) $(MODULE_OBJS)
makeiso:
	cp kernel.bin ../iso/boot/grub
	grub-mkrescue -o ~/myos.iso ../iso
	cp ~/myos.iso ./
runkernel:
	"qemu-system-i386" -cpu 486 -kernel kernel.bin -serial stdio -m 256M
run:
	qemu-system-i386 -cdrom myos.iso
bochs:
	bochs -f bochsout.bsx -q
install:
	sudo cp kernel.bin \boot\grub
debug:
	qemu-system-i386 -cdrom myos.iso -s -S &
	gdb kernel.bin -tui -x ..\debug.gdb
arm:
	make -C arch\armv7-a
clean-prg:
	make -C ..\userland\libc clean
	make -C module clean
make-prg:
	make -C ..\userland\libc
	make -C module
