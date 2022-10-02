target remote localhost:1234
#b __process_destroy
#b sys_waitpid
#b vmm_createDirectory
#b sys_exec
#b vfs_read
#b exec_init
b pmml_allocPages
c
