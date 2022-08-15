#ifndef ATAPI_H
#define ATAPI_H
/* Yeah i do some commentaries about that, it's the kernel ATAPI driver used for detection the media like
hard drive,CDROM, it's need for the FS and other comoonents */
/* Init ATAPI then try to detect media's */
void ata_init();
/* Reset ATAPI drive */
void ata_reset();
/* Probe our ATA drive */
int ata_probe(int unit, int *nblocks, int *blocksize, char *name);
/* Read from ATA drive */
int ata_read(int unit, void *buffer,int nblocks,int offset);
/* Write to ATA drive */
int ata_write(int unit, const void *buffer,int nblocks,int offset);
// Now ATAPI
/* Probe our ATAPI drive */
int atapi_probe(int unit,int *nblocks,int *blocksize,char *name);
/* Read from ATAPI */
int atapi_read(int unit,void *buffer,int nblocks,int offset);
#endif