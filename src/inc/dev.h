#ifndef DEV_H
#define DEV_H
typedef struct dev {
    char *name;
    int buffer_sizeMax;
    void (*write)(void *buffer,int size);
    void (*read)(char *buffer,int size);
    struct dev *next;
    struct dev *prev;
} dev_t;
void dev_init();
void dev_add(dev_t *dev);
dev_t *dev_find(char *name);
void dev_remove(int id);
#endif