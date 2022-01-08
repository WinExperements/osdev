CCPARAMS=-m32 -nostdlib -fno-builtin -fno-exceptions -fno-leading-underscore -Iinc -c
ASPARAMS = --32
LDPARAMS = -melf_i386

objects = src/x86/start.o src/x86/io.o src/x86/printk.o src/main.o src/panic.o src/x86/gdts.o src/x86/gdt.o src/x86/arch.o src/x86/interrupts.o src/x86/interrupt.o src/x86/idt.o src/mm.o
%.o: %.c
	cc $(CCPARAMS) -o $@ $<
%.o: %.s
	as $(ASPARAMS) -o $@ $<
%.o: %.asm
	nasm -f elf -o $@ $<
all: $(objects)
	ld $(LDPARAMS) -T linker.ld $(objects) -o  mykernel.bin

iso:
	mkdir iso
	mkdir iso/boot
	mkdir iso/boot/grub
	cp mykernel.bin iso/boot/mykernel.bin
	echo 'set timeout=60'                      > iso/boot/grub/grub.cfg
	echo 'set default=0'                     >> iso/boot/grub/grub.cfg
	echo ''                                  >> iso/boot/grub/grub.cfg
	echo 'menuentry "Helin OS" {' >> iso/boot/grub/grub.cfg
	echo '  multiboot /boot/mykernel.bin'    >> iso/boot/grub/grub.cfg
	echo '  boot'                            >> iso/boot/grub/grub.cfg
	echo '}'                                 >> iso/boot/grub/grub.cfg
	grub-mkrescue --output=mykernel.iso iso
	rm -rf iso
