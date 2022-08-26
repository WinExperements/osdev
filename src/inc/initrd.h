#ifndef INITRD_H
#define INITRD_H
#include<typedefs.h>
#include<vfs.h>
typedef struct
{
   uint32_t nfiles; // The number of files in the ramdisk.
} initrd_header_t;

typedef struct
{
   uint8_t magic;     // Magic number, for error checking.
   s8 name[64];  // Filename.
   uint32_t offset;   // Offset in the initrd that the file starts.
   uint32_t length;   // Length of the file.
} initrd_file_header_t;
vfs_node_t *initrd_getRoot();
void initrd_init(void *start);
uint32_t initrd_getFS();
void initrd_mount(char *mountPath);
#endif