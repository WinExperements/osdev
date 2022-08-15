/** Virtual File System **/
#include<vfs.h>
#include<terminal.h>
vfs_fs_t *fs_start;
void vfs_init() {
    printf("VFS: version 1.0\n");
}
void vfs_add(vfs_fs_t *fs) {
    if (!fs_start) {
        fs_start = fs;
        return;
    } else {
        vfs_fs_t *ffs = fs_start;
        while(ffs->next) {
            ffs = ffs->next;
        }
        ffs->next = fs; 
    }
}