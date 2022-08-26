/** Virtual File System **/
#include<vfs.h>
#include<terminal.h>
#include<mstring.h>
#include<dev.h>
vfs_fs_t *fs_start;
vfs_node_t *fs_root;
void vfs_init() {
    printf("VFS: version 1.0\n");
}
void vfs_addFS(vfs_fs_t *fs) {
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
void vfs_read(vfs_node_t *node,uint32_t offset,uint32_t how,void *buf) {
    if (!node) {
        printf("vfs: file seems broken, or you trying to read don't exists file\n");
        return;
    } else {
        dev_t *d;
        if ((d = dev_find(node->name)) != NULL) {
            d->read(buf,how);
            return;
        }
        node->read(node,offset,how,buf);
    }
}
void vfs_write(vfs_node_t *node,uint32_t offset,uint32_t how,void *buf) {
    if (!node) {
        printf("vfs: file seems broken, or you trying to write to don't exists file\n");
        return;
    } else {
        dev_t *d;
        if ((d = dev_find(node->name)) != NULL) {
            d->write(buf,how);
            return;
        }
        node->write(node,offset,how,buf);
    }
}
void vfs_open(vfs_node_t *node,bool r,bool w) {
    if (!node) {
        if (!w) {
            printf("vfs: file seems broken, or you trying to open don't exists file\n");
            return;
        }
    } else {
        node->open(node,w,r);
    }
}
void vfs_close(vfs_node_t *node) {
    if (!node || !node->close) {
        printf("vfs: file seems broken, or you trying to read close, didn't oppened file\n");
        return;
    } else {
        node->close(node);
    }
}
struct dirent *vfs_readdir(vfs_node_t *in,uint32_t index) {
    if (!in) return NULL;
    return in->readdir(in,index);
}
vfs_node_t *vfs_finddir(vfs_node_t *in,char *name) {
    if (!in || !name) return NULL;
    return in->finddir(in,name);
}
vfs_node_t *vfs_getRoot() {
    return fs_root;
}
void vfs_mount(vfs_node_t *root,char *mountPoint) {
    if (!root) {
        printf("%s: filesystem root are null!",__func__);
        return;
    }
    if (!mountPoint || mountPoint[0] != '/') {
        printf("%s: mount point are invalid",__func__);
    }
    if (strcmp(mountPoint,"/")) {
        fs_root = root;
    }
}
vfs_node_t *vfs_creat(vfs_node_t *in,char *name,int flags) {
    if (!in || !in->creat) {
        printf("mkdir: broken target\n");
        return NULL;
    }
    return in->creat(in,name,flags);
}
void vfs_changeDir(vfs_node_t *to) {
    if (!to) {
        printf("cd: invalid directory\n");
        return;
    }
    fs_root = to;
}
void vfs_truncate(vfs_node_t *node,int size) {
    if (!node || !node->truncate) {
        node->truncate(node,size);
    }
}
vfs_node_t *vfs_find(char *path) {
    printf("[vfs]: find %s\n",path);
    return NULL;
}
void vfs_readBlock(vfs_node_t *node,int blockN,int how,void *buff) {
    if (!node || !node->readBlock) return;
    node->readBlock(node,blockN,how,buff);
}
void vfs_writeBlock(vfs_node_t *node,int blockN,int how,void *buff) {
    if (!node || !node->writeBlock) return;
    node->writeBlock(node,blockN,how,buff);
}
void vfs_ioctl(vfs_node_t *node,int request,void *argp) {
    if (!node || !node->ioctl) return;
    node->ioctl(node,request,argp);
}