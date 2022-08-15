#ifndef DEV_H
#define DEV_H
typedef struct dev {
    char *name;
    int buffer_sizeMax;
    void (*write)(void *buffer,int size);
    void (*read)(void *buffer,int size);
    struct dev *next;
    struct dev *prev;
} dev_t;
void dev_init();
void dev_add(dev_t *dev);
void dev_write(int id,char *buffer,int size);
void dev_read(int id,char *buffer,int size);
void dev_remove(int id);
#endif