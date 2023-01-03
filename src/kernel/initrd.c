#include<initrd.h>
#include<terminal.h>
#include<vfs.h>
#include<mm/pmm.h>
#include<mstring.h>
initrd_header_t *header;
initrd_file_header_t *file_headers;
vfs_node_t *initrd_root;
vfs_node_t *initrd_dev;
vfs_node_t *initrd_nodes;
void initrd_read(vfs_node_t *node,uint64_t offset,uint64_t how,void *buf);
struct dirent *initrd_readdir(vfs_node_t *in,uint32_t index);
vfs_node_t *initrd_finddir(vfs_node_t *root,char *name);
struct dirent dirent;
void initrd_init(void *start) {
    header = (initrd_header_t *)start;
    vfs_fs_t *initrd_fs = (vfs_fs_t *)pmml_alloc(true);
    // setup our VFS
    initrd_fs->fs_name = "initrd";
    vfs_addFS(initrd_fs);
    file_headers = (initrd_file_header_t *)((int)start+sizeof(initrd_header_t));
    initrd_root = (vfs_node_t *)pmml_alloc(true);
    initrd_root->name = "root";
    initrd_root->mask = initrd_root->uid = initrd_root->guid = initrd_root->inode = initrd_root->size = 0;
    initrd_root->flags = VFS_DIRECTORY;
    initrd_root->read = initrd_read;
    initrd_root->finddir = initrd_finddir;
    initrd_root->readdir = initrd_readdir;
    initrd_dev = (vfs_node_t *)pmml_alloc(true);
    initrd_dev->name = "dev";
    initrd_dev->flags = VFS_DIRECTORY;
    initrd_dev->read = initrd_read;
    initrd_dev->finddir = initrd_finddir;
    initrd_dev->readdir = initrd_readdir;
    initrd_nodes = (vfs_node_t *)pmml_allocPages(((sizeof(vfs_node_t) * header->nfiles)/4096)+1,true);
    for (uint32_t i = 0; i < header->nfiles; i++) {
        file_headers[i].offset +=(uint32_t)start;
        initrd_nodes[i].name = file_headers[i].name;
        initrd_nodes[i].size = file_headers[i].length;
        initrd_nodes[i].inode = i;
        initrd_nodes[i].read = initrd_read;
        initrd_nodes[i].finddir = initrd_finddir;
        initrd_nodes[i].readdir = initrd_readdir;
    }
    printf("initrd: files: %d\n",header->nfiles);
}
void initrd_read(vfs_node_t *node,uint64_t offset,uint64_t how,void *buf) {
    initrd_file_header_t head = file_headers[node->inode];
    if (offset > head.length) {
        return;
    }
    if (offset+how > head.length) {
        how = head.length-offset;
    }
    memcpy(buf,(uint8_t *)(head.offset+offset),how);
}
uint32_t initrd_getFS() {
    return 0;
}
struct dirent *initrd_readdir(vfs_node_t *in,uint32_t index) {
    if (in == initrd_root && index == 0) {
        strcpy(dirent.name,"dev");
        dirent.name[3] = 0;
        return &dirent;
    }
    if (index-1 >= header->nfiles) {
        return NULL;
    }
    strcpy(dirent.name,initrd_nodes[index-1].name);
    dirent.name[strlen(initrd_nodes[index - 1].name)] = 0;
    dirent.node = initrd_nodes[index-1].inode;
    return &dirent;
}
vfs_node_t *initrd_finddir(vfs_node_t *root,char *name) {
    if (root == initrd_root && strcmp(name,"dev")) {
        return initrd_dev;
    }
    int i;
    for (i = 0; i < header->nfiles; i++) {
        if (strcmp(name,initrd_nodes[i].name)) {
            return &initrd_nodes[i];
        }
    }
    return NULL;
}
vfs_node_t *initrd_getRoot() {
    return initrd_root;
}
void initrd_mount(char *mountPath) {
    vfs_mount(initrd_root,NULL,mountPath);
    printf("initrd: mounted as %s\n",mountPath);
}