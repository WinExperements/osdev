/** Virtual File System **/
#include<vfs.h>
#include<terminal.h>
#include<mstring.h>
#include<dev.h>
#include<arch.h>
#include<serial.h>
vfs_fs_t *fs_start;
vfs_node_t *fs_root;
vfs_node_t *vfs_find_impl(vfs_node_t *start,char *path);
void vfs_init() {
#ifdef DEBUG
    write_serialString("vfs: initialized\r\n");
#endif
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
        node->read(node,offset,how,buf);
    }
}
void vfs_write(vfs_node_t *node,uint32_t offset,uint32_t how,void *buf) {
    if (!node) {
        printf("vfs: file seems broken, or you trying to write to don't exists file\n");
        return;
    } else {
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
void vfs_mount(vfs_fs_t *fs,char *mountPoint) {
    if (!fs) {
        printf("%s: filesystem are null!",__func__);
        return;
    }
    if (!mountPoint || mountPoint[0] != '/') {
        printf("%s: mount point are invalid",__func__);
    }
    if (strcmp(mountPoint,"/")) {
	if (!fs->mount) {
                printf("%s: filesystem mount function are not defined\n");
                return;
        }
	vfs_node_t *root = fs->mount(NULL,NULL);
        if (!root) {
                printf("%s: filesystem mount failed\n");
                return;
        }
        fs_root = root;
    } else {
        vfs_node_t *mount_point = vfs_find(mountPoint);
	if (!mount_point) {
		printf("%s: no such file or directory\n",mountPoint);
		return;
	} else if ((mount_point->flags & 0x7) != VFS_DIRECTORY) {
		printf("%s: not directory\n",mountPoint);
		return;
	}
	// call the actual mount function(if present)
	if (!fs->mount) {
		printf("%s: filesystem mount function are not defined\n");
		return;
	}
	vfs_node_t *root = fs->mount(mount_point,NULL);
        if (!root) {
                printf("%s: filesystem mount failed\n");
                return;
        }
	// replace all I/O functions with the root directory functions
	mount_point->read = root->read;
	mount_point->write = root->write;
	mount_point->open = root->open;
	mount_point->close = root->close;
	mount_point->finddir = root->finddir;
	mount_point->readdir = root->readdir;
	mount_point->creat = root->creat;
	mount_point->truncate = root->truncate;
	mount_point->readBlock = root->readBlock;
	mount_point->writeBlock = root->writeBlock;
	mount_point->ioctl = root->ioctl;
   }
}
vfs_node_t *vfs_creat(vfs_node_t *in,char *name,int flags) {
    if (!in || !in->creat) {
        printf("touch: broken target\n");
        return NULL;
    }
    return in->creat(in,name,flags);
}
void vfs_truncate(vfs_node_t *node,int size) {
    if (!node || node->truncate != 0) {
        node->truncate(node,size);
    }
}
vfs_node_t *vfs_find(char *path) {
    if (path[0] == '/') {
        return vfs_find_impl(vfs_getRoot(),path);
    } else {
        vfs_node_t *workDir = process_getProcess(process_getCurrentPID())->workDir;
        if (!workDir) {
            printf("vfs_find: Seems your process are broken, cannot find work dir\n");
            return NULL; // return
        }
        if (strcmp(path,"..")) {
            return workDir->prev;
        }
        return vfs_find_impl(workDir,path);
    }
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
vfs_node_t *vfs_find_impl(vfs_node_t *start,char *path) {
    char *t = strtok(path,"/");
    vfs_node_t *node = start;
    while(t) {
        node = vfs_finddir(node,t);
        if (!node) return NULL;
        t = strtok(NULL,"/");
    }
    return node;
}
void vfs_node_path(vfs_node_t *node,char *path,int size) {
    if (node == fs_root) {
        if (size > 1) {
            path[0] = '/';
            path[1] = '\0';
            return;
        } else {
            return;
        }
    } else {
        char target_path[128];
        vfs_node_t *n = node;
        int char_index = 127;
        while (n != NULL) {
            int len = strlen(n->name);
            char_index-=len;
            if (char_index < 2) {
                return;
            }
            if (n->prev != NULL) {
                strcpy(target_path+char_index,n->name);
                char_index-=1;
                target_path[char_index] = '/';
            }
            n = n->prev;
        }
        int len = 127-char_index;
        if (size < len) {
            return;
        }
        strcpy(path,target_path+char_index+1);
    }
}
vfs_fs_t *vfs_findFS(char *name) {
    vfs_fs_t *start = fs_start;
    while(start != NULL) {
        if (strcmp(start->fs_name,name)) {
            return start;
        }
        start = start->next;
    }
    return NULL;
}
void *vfs_mmap(struct vfs_node *node,int addr,int size,int offset,int flags) {
	if (!node || !node->mmap) return NULL;
	return node->mmap(node,addr,size,offset,flags);
}
