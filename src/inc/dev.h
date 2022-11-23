#ifndef DEV_H
#define DEV_H
#include<typedefs.h>
#include<vfs.h>
typedef struct dev {
    char *name;
    int buffer_sizeMax;
    void (*write)(struct vfs_node *node,uint32_t offset,uint32_t how,void *buf);
    void (*read)(struct vfs_node *node,uint32_t offset,uint32_t how,void *buf);
    void *(*mmap)(struct vfs_node *node,int addr,int size,int offset,int flags);
    struct dev *next;
    struct dev *prev;
} dev_t;
void dev_init();
void dev_add(dev_t *dev);
dev_t *dev_find(char *name);
void dev_remove(int id);
#endif
