/** Root file system **/
#include<vfs.h>
#include<typedefs.h>
#include<mm/pmm.h>
#include<mstring.h>
#include<mm.h>
#include<terminal.h>
vfs_node_t *root;
vfs_fs_t *rootfs_fs;
struct dirent rootfs_dirent;
int files = 0;
void rootfs_read(vfs_node_t *node,uint32_t offset,uint32_t how,void *buf);
struct dirent *rootfs_readdir(vfs_node_t *in,uint32_t index);
vfs_node_t *rootfs_finddir(vfs_node_t *root,char *name);
vfs_node_t *rootfs_creat(vfs_node_t *node,char *name,int flags);
void rootfs_write(vfs_node_t *node,uint32_t offset,uint32_t how,void *buf);
void rootfs_truncate(vfs_node_t *in,int size);
void rootfs_close(vfs_node_t *in);
char **data;
void rootfs_init() {
    // we need to register new FS
    rootfs_fs = pmml_alloc(true);
    rootfs_fs->fs_name = "rootfs";
    root = pmml_alloc(true);
    root->name = "/";
    root->flags = VFS_DIRECTORY;
    root->read = rootfs_read;
    root->readdir = rootfs_readdir;
    root->finddir = rootfs_finddir;
    root->creat = rootfs_creat;
    root->write = rootfs_write;
    root->truncate = rootfs_truncate;
    root->close = rootfs_close;
    vfs_addFS(rootfs_fs);
    data = pmml_alloc(true);
}
void rootfs_read(vfs_node_t *node,uint32_t offset,uint32_t how,void *buf) {
    if ((node->flags & 0x7) != VFS_DIRECTORY) {
        memcpy(buf,data[node->inode],how);
    }
}
struct dirent *rootfs_readdir(vfs_node_t *in,uint32_t index) {
    // find node
    uint32_t i = 0;
    vfs_node_t *n = in->first_child;
    while(n != NULL) {
        if (i == index) {
            memset(&rootfs_dirent,0,sizeof(rootfs_dirent));
            rootfs_dirent.node = n->inode;
            strcpy(rootfs_dirent.name,n->name);
            return &rootfs_dirent;
        }
        n = n->next_child;
        ++i;
    }
    return NULL;
}
vfs_node_t *rootfs_finddir(vfs_node_t *root,char *name) {
    vfs_node_t *n = root->first_child;
    while(n != NULL) {
        if (strcmp(n->name,name)) {
            return n;
        }
        n = n->next_child;
    }
    return NULL;
}
vfs_node_t *rootfs_creat(vfs_node_t *node,char *name,int flags) {
    vfs_node_t *newDir = pmml_alloc(true);
    newDir->name = pmml_alloc(true);
    strcpy(newDir->name,name);
    newDir->name[strlen(name)] = 0;
    newDir->flags = flags;
    newDir->read = rootfs_read;
    newDir->readdir = rootfs_readdir;
    newDir->finddir = rootfs_finddir;
    newDir->creat = rootfs_creat;
    newDir->write = rootfs_write;
    newDir->truncate = rootfs_truncate;
    newDir->close = rootfs_close;
    newDir->next_child = NULL;
    newDir->prev = node;
    int i = 1;
    if (node->first_child == NULL) {
        node->first_child = newDir;
        return newDir;
    }
    vfs_node_t *n = node->first_child;
    while(n->next_child != NULL) {
        n = n->next_child;
        i++;
    }
    newDir->inode = i;
    n->next_child = newDir;
    files++;
    return newDir;
}
void rootfs_mount(char *to) {
    vfs_mount(root,to);
}
void rootfs_write(vfs_node_t *node,uint32_t offset,uint32_t how,void *buf) {
    char *buff = pmml_allocPages((how/4096)+1,true);
    memcpy(buff+offset,buf+offset,how-offset);
    data[node->inode] = buff;
    node->size = how;
}
void rootfs_truncate(vfs_node_t *node,int size) {
    node->size = size;
}
void rootfs_close(vfs_node_t *node) {}