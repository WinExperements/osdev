# IMPORTANT!!
Project halted, because I have big problems with the PMM, see issues for more details.
Maybe I don't return to this, sorry
# About
A hobby operation system written in C and assembly for X86 i386 PC.<br>
You can use source of this project :)<br>
# Features
Current list of features and released components:
- [x] Global description table
- [x] Interrupts
- [x] Memory Management, PMM
- [x] Multitasking
- [x] Userspace program loading
- [x] Multiboot structure loading
- [x] User space [in this file](arch/x86/gdtfl.asm)
- [x] Keyboard driver in user space
- [x] Working syscall interface
- [x] Base libc
- [x] VGA
- [x] Virtual memory
- [x] Minimal userspace shell

# How to build?
- Install any cross-compiler
- Install ```grub-common xorriso gcc make nasm```
- If you don't use Linux, install it on VM to build the OS.
- Go to src folder
- Type "make" this build only kernel
- Type "make make-prg" to build userspace dynamic libc and some programs.
- To create ISO type "make makeiso"
# TODO
- More kernel parameters
- Functionally libc
- Add support for mount point, block devices,ramdisk, FAT filesystem, module and shared objects loading
# Changelog
See releases.
