/* Character device communication */
#include<dev.h>
#include<typedefs.h>
#include<terminal.h>
#include<mstring.h>
#include<vfs.h>
#include<mm/pmm.h>
dev_t *device;
vfs_node_t *dev_dir;
void dev_init() {
    device = NULL;
    dev_dir = vfs_finddir(vfs_getRoot(),"dev");
    if (!dev_dir) {
        printf("no /dev");
        return;
    }
}
void dev_add(dev_t *dev) {
    vfs_node_t *fil = vfs_creat(dev_dir,dev->name,0);
    vfs_truncate(fil,dev->buffer_sizeMax);
    if (!device) {
        device = dev;
    } else {
        dev_t *de = device;
        while(de->next != NULL) {
            de = de->next;
        }
        de->next = dev;
    }
}
dev_t *dev_find(char *name) {
    dev_t *d = device;
    while(d != NULL) {
        if (strcmp(d->name,name)) {
            return d;
        }
        d = d->next;
    }
    return NULL;
}
void dev_remove(int id) {}