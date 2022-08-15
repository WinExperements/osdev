[GLOBAL arch_poweroff]
[GLOBAL syscall_call]
arch_poweroff:
	ret
syscall_call:
	int 0x80
	ret