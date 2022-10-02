all:
	gcc -g -m32 -nodefaultlibs -nostartfiles -I../libc -c main.c
	gcc -g -m32 -nodefaultlibs -nostartfiles -I../libc -c test.c
	ld -melf_i386 -nodefaultlibs -nostartfiles -T../user.ld main.o ../libc/libcc.a -o init
	ld -melf_i386 -nodefaultlibs -nostartfiles -T../user.ld test.o ../libc/libcc.a -o test
	gcc -g -m32 -nodefaultlibs -nostartfiles -I../libc -c in.c
	ld -melf_i386 -nodefaultlibs -nostartfiles -T../user.ld in.o ../libc/libcc.a -o in
	cp init ../../iso/boot
	cp test ../../iso/boot/
	#cp in ../../iso/boot/init
clean:
	rm -rf *.o init test in
