#include<terminal.h>
#include <io.h>
#include <mm/pmm.h>
#include <typedefs.h>
#include <vfs.h>
#include <dev.h>
#include "mbr.h"
#include <mstring.h>
static int global_part_id = 0;
// define our module name in .modname section that needed for our module loader!
__attribute__((section(".modname"))) char *name = "mbr";

static mbr_t m_mbr;

static void mbr_dev_read(vfs_node_t *node,int blockNo,int how,void *buf) {
    if (node->device == NULL) {
        printf("MBR: %s: no device pointer in inode!\n",node->name);
        return;
    }
    mbr_dev_t *dev = (mbr_dev_t *)node->device;
    if (dev->lba_start == 0) return;
    int off = dev->lba_start;
    off+=blockNo;
    printf("MBR lba: %d, blockNo: %d\n",off,blockNo);
    vfs_readBlock((vfs_node_t *)dev->harddrive_addr,off,how,buf);
}

static void mbr_dev_write(vfs_node_t *node,uint64_t offset,uint64_t how,void *buf) {
    /*if (node->device == NULL) {
        printf("MBR: %s: no device pointer in inode!\n",node->name);
        return;
    }
    mbr_dev_t *dev = (mbr_dev_t *)node->device;
    int off = 512*dev->mbr->partitions[dev->part_index].lba_first_sector;
    off+=offset;
    vfs_write((vfs_node_t *)dev->harddrive_addr,off,how,buf);*/
}

static void mbr_registerDevice(vfs_node_t *harddrive,int part_index,mbr_t *mbr) {
    mbr_dev_t *dev = pmml_alloc(true);
    dev->harddrive_addr = (int)harddrive;
    dev->part_index = part_index;
    dev->lba_start = mbr->partitions[part_index].lba_first_sector;
    dev_t *d = pmml_alloc(true);
    d->name = pmml_alloc(true);
    strcpy(d->name,harddrive->name);
    d->name[3] = 'p';
    d->name[4]  =  global_part_id+'0';
    d->name[5] = '\0';
    d->write = mbr_dev_write;
    d->readBlock = mbr_dev_read;
    d->device = dev;
    dev_add(d);
    global_part_id++;
}

static void mbr_parseMbr(vfs_node_t *harddrive,uint32_t extPartSector,mbr_t mbr) {
    if (mbr.signature[0] == 0x55 && mbr.signature[1] == 0xAA) {
        for (int i = 0; i < 4; i++) {
            if (mbr.partitions[i].sector_count != 0) { //active partiton
                    int off = extPartSector;
                    mbr.partitions[i].lba_first_sector += off;
                    printf("MBR: Partiton %d start: %d, size(in sectors): %d, offset: %d\n",i,mbr.partitions[i].lba_first_sector,mbr.partitions[i].sector_count,off);
                    mbr_registerDevice(harddrive,i,&mbr);
                    if (mbr.partitions[i].type == 0x5 || mbr.partitions[i].type == 0xf) {
                        printf("MBR: Extended partiton, parsing\n");
                        mbr_t *sec_mbr = pmml_alloc(true);
                        vfs_readBlock(harddrive,(mbr.partitions[i].lba_first_sector),512,sec_mbr);
                        mbr_parseMbr(harddrive,mbr.partitions[i].lba_first_sector,*sec_mbr);
                        printf("MBR: Extended partition parsing done\n");
                        pmml_free(sec_mbr);
                    }
            } else {
                printf("MBR: Partition %d inactive\n",i);
            }
        }
    } else {
        printf("MBR: No valid partition table found\n");
        printf("Returned signature: %x, %x\n",mbr.signature[0],mbr.signature[1]);
    }
}

static void module_main() {
    // Currently support only on master drive
    vfs_node_t *harddrive = vfs_find("/dev/hda");
    if (!harddrive) {
        printf("MBR: cannot open hard drive A!\n");
        return; // exit
    }
    // read mbr_t structure
    printf("MBR: Reading 512 bytes\n");
    printf("MBR structure at: %x\n",&m_mbr);
    printf("MBR: Reading block\n");
    vfs_readBlock(harddrive,0,512,&m_mbr);
    printf("MBR: Parsing readed data\n");
    mbr_parseMbr(harddrive,0,m_mbr);
}
mbr_t *mbr_getMBR() {
    return &m_mbr;
}
