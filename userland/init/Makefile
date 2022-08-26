all:
	i686-elf-gcc -m32 -nodefaultlibs -nostartfiles -I..\libc -c main.c
	i686-elf-gcc -m32 -nodefaultlibs -nostartfiles -T..\user.ld main.o ..\libc\libcc.a -o init
	copy init ..\..\iso\boot