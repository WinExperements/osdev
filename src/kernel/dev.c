/* Character device communication */
#include<dev.h>
#include<typedefs.h>
#include<terminal.h>
dev_t *device;
void dev_init() {
    device = NULL;
}
void dev_add(dev_t *dev) {
    if (device = NULL) {
        device = dev;
    } else {
        dev_t *de = device;
        while(de->next != NULL) {
            de = de->next;
        }
        de->next = dev;
    }
}
void dev_write(int id,char *buffer,int size) {
    dev_t *dev = device;
    for (int i = 0; i < id; i++) {
        dev = dev->next;
    }
    if (dev->name == NULL) {
        printf("%s: no such device %d\n",__func__,id);
        return;
    }
    dev->write(buffer,size);
}
void dev_read(int id,char *buffer,int size) {}
void dev_remove(int id) {}