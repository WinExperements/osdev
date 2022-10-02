ARCH=x86
CCOPTIONS = -m32 -ggdb -std=gnu99 -ffreestanding -Wall -Wextra -Wno-unused-parameter -Wint-to-pointer-cast -Wsign-compare -nostdlib -march=i486 -mtune=i486 -fno-stack-protector -DLEGACY_TERMINAL
ASPARAMS =
CCPATH = 
OBJECTS = arch/$(ARCH)/boot.o arch/$(ARCH)/terminal.o kernel/kernel.o arch/$(ARCH)/io.o arch/$(ARCH)/serial.o arch/$(ARCH)/arch.o arch/$(ARCH)/gdt.o arch/$(ARCH)/gdtfl.o arch/x86/interrupts.o arch/x86/interruptsasm.o mm/mm.o mm/pmm.o arch/$(ARCH)/vmm.o kernel/mstring.o kernel/process.o kernel/syscall.o kernel/keyboard.o arch/x86/pic.o kernel/elf.o font.o kernel/dev.o ipc/message.o kernel/vfs.o kernel/initrd.o kernel/module.o kernel/kshell.o kernel/rootfs.o kernel/terminal.o lib/clist.o kernel/arch.o
MODULE_OBJS = module/atapi/atapi.o
%.o: %.c
	@echo [CC] $<
	@$(CCPATH)gcc $(CCOPTIONS) -Iinc -Imodule -c -o $@ $<
%.o: %.s
	@echo [ASM] $<
	@$(CCPATH)as --32 -g -o $@ $<
%.o: %.asm
	@echo [NASM] $<
	@nasm -f elf32 -F dwarf -g -o $@ $<
%.o: %.psf
	@echo [OBJCOPY] $<
	@$(CCPATH)objcopy -O elf32-i386 -B i386 -I binary $< $@
all: $(OBJECTS) $(MODULE_OBJS)
	@echo [LD] kernel.bin
	@$(CCPATH)ld -melf_i386 -T arch/$(ARCH)/linker.ld -Map=kernel.map -o kernel.bin $(OBJECTS) $(MODULE_OBJS)
clean:
	rm -rf $(OBJECTS) $(MODULE_OBJS) kernel.bin kernel.map
	rm -rf ../iso/boot/grub/kernel.bin ../iso/boot/init ../iso/boot/test
makeiso:
	cp kernel.bin ../iso/boot/grub
	grub-mkrescue -o ~/myos.iso ../iso
	cp ~/myos.iso ./
runkernel:
	"qemu-system-i386" -cpu 486 -kernel kernel.bin -serial stdio -m 256M
run:
	qemu-system-i386 -cpu 486 -cdrom myos.iso
bochs:
	bochs -f bochsout.bsx -q
install:
	sudo cp kernel.bin /boot/grub
debug:
	qemu-system-i386 -cdrom myos.iso -s -S &
	gdb kernel.bin -tui -x ../debug.gdb
arm:
	make -C arch/armv7-a
clean-prg:
	make -C ../userland/libc clean
	make -C ../userland/init clean
make-prg:
	make -C ../userland/libc
	make -C ../userland/init
