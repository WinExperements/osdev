#ifndef VFS_H
#define VFS_H
#include<typedefs.h>
typedef struct vfs_node {
    char *name;
    uint32_t mask;
    uint32_t guid;
    uint32_t uid;
    uint32_t flags;
    uint32_t inode;
    uint32_t size;
} vfs_node_t;
typedef struct vfs_fs {
    char *fs_name;
    void (*read)(vfs_node_t *node,uint32_t how,uint32_t *buf);
    void (*write)(vfs_node_t *node,uint32_t how,uint32_t *buf);
    void (*open)(vfs_node_t *node);
    void (*close)(vfs_node_t *node);
    struct vfs_fs *next;
} vfs_fs_t;
void vfs_init();
void vfs_addFS(vfs_fs_t *fs);
void vfs_read(vfs_node_t *node,uint32_t how,uint32_t *buf);
#endif