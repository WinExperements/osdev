# About
A hobby operation system written in C and assembly.<br>
You can use source of this project :)<br>
Here the screnshot:<br>
![alt text](screenshot.png)
# Features
Current list of features and released components:
- [x] Global description table
- [x] Interrupts
- [x] Memory Management, PMM
- [x] Multitasking
- [x] Module Loading
- [x] Multiboot structure loading
- [x] User space [in this file](arch/x86/gdtfl.asm)
- [x] Keyboard driver in user space
- [x] Working syscall interface
- [x] Base libc
- [x] VGA
- [x] Virtual memory

# How to build?
- Install any cross-compiler
- Install ```grub-common xorriso grub-efi-ia32```
- If you don't use Windows, then replace all ```\\``` to ```/``` in Makefile
- Type "make" or if you use Windows, type "mingw32-make"
- If you in Windows you need to install any Linux distribution using virtual machine to make ISO, if not just type "make makeiso"
# Bad news
- The Operation System may don't support x86_64 CPU
- Image creation(.iso) available only in linux
# TODO
- More kernel parameters
- Functionally libc
# Changelog of 
## Terminal
- Added support to legacy terminal, if you need to remove this, just remove ```-DLEGACY_TERMINAL``` in Makefile, and add ```| (1 << 2)``` to ```arch\x86\boot.s```
## Other
- Added some unimplemented functions to load modules
## Multitasking
- Added functional to process_wait function