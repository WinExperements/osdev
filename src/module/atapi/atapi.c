#include <terminal.h>
#include<io.h>
#include<atapi/atapi.h>
#include<serial.h>
#include<dev.h>
#include<mm/pmm.h>
#include<interrupts.h>
// === Internal functions here ===
typedef struct {
	uint16_t flags;
	uint16_t unused1[9];
	char     serial[20];
	uint16_t unused2[3];
	char     firmware[8];
	char     model[40];
	uint16_t sectors_per_int;
	uint16_t unused3;
	uint16_t capabilities[2];
	uint16_t unused4[2];
	uint16_t valid_ext_data;
	uint16_t unused5[5];
	uint16_t size_of_rw_mult;
	uint32_t sectors_28;
	uint16_t unused6[38];
	uint64_t sectors_48;
	uint16_t unused7[152];
} __attribute__((packed)) ata_identify_t;
typedef struct {
	uint8_t reserved;
	uint8_t channel;
	uint8_t drive;
	uint8_t type;
	uint16_t signature;
	uint16_t sup;
	uint32_t cmd_sets;
	uint32_t size;
	char model[41];
	uint16_t base;
	uint16_t ctrl;
	uint16_t nein;
	int slave;
	ata_identify_t identify;
} ata_device_t;
#define ATA_SR_BSY     0x80
#define ATA_SR_DRDY    0x40
#define ATA_SR_DF      0x20
#define ATA_SR_DSC     0x10
#define ATA_SR_DRQ     0x08
#define ATA_SR_CORR    0x04
#define ATA_SR_IDX     0x02
#define ATA_SR_ERR     0x01

#define ATA_ER_BBK      0x80
#define ATA_ER_UNC      0x40
#define ATA_ER_MC       0x20
#define ATA_ER_IDNF     0x10
#define ATA_ER_MCR      0x08
#define ATA_ER_ABRT     0x04
#define ATA_ER_TK0NF    0x02
#define ATA_ER_AMNF     0x01

#define ATA_CMD_READ_PIO          0x20
#define ATA_CMD_READ_PIO_EXT      0x24
#define ATA_CMD_READ_DMA          0xC8
#define ATA_CMD_READ_DMA_EXT      0x25
#define ATA_CMD_WRITE_PIO         0x30
#define ATA_CMD_WRITE_PIO_EXT     0x34
#define ATA_CMD_WRITE_DMA         0xCA
#define ATA_CMD_WRITE_DMA_EXT     0x35
#define ATA_CMD_CACHE_FLUSH       0xE7
#define ATA_CMD_CACHE_FLUSH_EXT   0xEA
#define ATA_CMD_PACKET            0xA0
#define ATA_CMD_IDENTIFY_PACKET   0xA1
#define ATA_CMD_IDENTIFY          0xEC

#define ATAPI_CMD_READ       0xA8
#define ATAPI_CMD_EJECT      0x1B

#define ATA_IDENT_DEVICETYPE   0
#define ATA_IDENT_CYLINDERS    2
#define ATA_IDENT_HEADS        6
#define ATA_IDENT_SECTORS      12
#define ATA_IDENT_SERIAL       20
#define ATA_IDENT_MODEL        54
#define ATA_IDENT_CAPABILITIES 98
#define ATA_IDENT_FIELDVALID   106
#define ATA_IDENT_MAX_LBA      120
#define ATA_IDENT_COMMANDSETS  164
#define ATA_IDENT_MAX_LBA_EXT  200

#define IDE_ATA        0x00
#define IDE_ATAPI      0x01
 
#define ATA_MASTER     0x00
#define ATA_SLAVE      0x01

#define ATA_REG_DATA       0x00
#define ATA_REG_ERROR      0x01
#define ATA_REG_FEATURES   0x01
#define ATA_REG_SECCOUNT0  0x02
#define ATA_REG_LBA0       0x03
#define ATA_REG_LBA1       0x04
#define ATA_REG_LBA2       0x05
#define ATA_REG_HDDEVSEL   0x06
#define ATA_REG_COMMAND    0x07
#define ATA_REG_STATUS     0x07
#define ATA_REG_SECCOUNT1  0x08
#define ATA_REG_LBA3       0x09
#define ATA_REG_LBA4       0x0A
#define ATA_REG_LBA5       0x0B
#define ATA_REG_CONTROL    0x0C
#define ATA_REG_ALTSTATUS  0x0C
#define ATA_REG_DEVADDRESS 0x0D
ata_device_t ata_primary_master   = {.base = 0x1F0, .ctrl = 0x3F6, .slave = 0};
ata_device_t ata_primary_slave    = {.base = 0x1F0, .ctrl = 0x3F6, .slave = 1};
ata_device_t ata_secondary_master = {.base = 0x170, .ctrl = 0x376, .slave = 0};
ata_device_t ata_secondary_slave  = {.base = 0x170, .ctrl = 0x376, .slave = 1};
void ata_create_device(bool hda);
void ata_vdev_read(struct vfs_node *node,uint32_t offset,uint32_t how,void *buf);
void ata_vdev_write(struct vfs_node *node,uint32_t offset,uint32_t how,void *buf);
char ata_start_char = 'a';
char ata_cdrom_char = 'a';
void ata_io_wait(ata_device_t *dev) {
	io_readPort(dev->base+ATA_REG_ALTSTATUS);
	io_readPort(dev->base+ATA_REG_ALTSTATUS);
	io_readPort(dev->base+ATA_REG_ALTSTATUS);
	io_readPort(dev->base+ATA_REG_ALTSTATUS);
}
int ata_status_wait(ata_device_t *dev,int timeout) {
	int status;
	if (timeout > 0) {
		int i =0;
		while((status = io_readPort(dev->base+ATA_REG_STATUS) & ATA_SR_BSY && (i < timeout))) i++;
	} else {
		while((status = io_readPort(dev->base+ATA_REG_STATUS)) & ATA_SR_BSY);
	}
	return status;	
}
int ata_wait(ata_device_t *dev,int advanced) {
	uint8_t status;
	ata_io_wait(dev);
	status = ata_status_wait(dev,-1); // wait forever!!
	if (advanced) {
		status = io_readPort(dev->base+ATA_REG_STATUS);
		if (status & ATA_SR_ERR) return true;
		if (status & ATA_SR_DF) return true;
		if (!(status & ATA_SR_DRQ)) return true;
	}
	return false;
}
void ata_soft_reset(ata_device_t *dev) {
	io_writePort(dev->ctrl,0x04);
	ata_io_wait(dev);
	io_writePort(dev->ctrl,0x00);
}
int ata_max_offset(ata_device_t *dev) {
	uint64_t sectors = dev->identify.sectors_48;
	if (!sectors) {
		sectors = dev->identify.sectors_28;
	}
	return sectors * 512;
}
// Initialization and detection code
bool ata_device_init(ata_device_t *dev) {
	io_writePort(dev->base+1,1);
	io_writePort(dev->ctrl,0);
	io_writePort(dev->base+ATA_REG_HDDEVSEL, 0xA0 | dev->slave << 4);
	ata_io_wait(dev);
	io_writePort(dev->base+ATA_REG_COMMAND,ATA_CMD_IDENTIFY);
	ata_io_wait(dev);
	io_readPort(dev->base+ATA_REG_COMMAND);
	bool err = ata_wait(dev,1);
	if (err) {
		printf("ATA: device error\n");
		return false;
	}
	uint16_t *buf = (uint16_t *)&dev->identify;
	for (int i = 0; i < 256; i++) {
		buf[i] = io_readPortW(dev->base);
	}
	uint8_t *ptr = (uint8_t *)&dev->identify.model;
	for (int i = 0; i < 39; i+=2) {
		uint8_t tmp = ptr[i+1];
		ptr[i+1] = ptr[i];
		ptr[i] = tmp;
	}
	return true;
}
int ata_device_detect(ata_device_t *dev) {
	ata_soft_reset(dev);
	ata_io_wait(dev);
	io_writePort(dev->base+ATA_REG_HDDEVSEL,0xA0 | dev->slave << 4);
	ata_io_wait(dev);
	ata_status_wait(dev,10000);
	unsigned char cl = io_readPort(dev->base + ATA_REG_LBA1);
	unsigned char ch = io_readPort(dev->base + ATA_REG_LBA2);
	if (ch == 0xff && cl == 0xff) {
		printf("ATA: drive not present\n");
		return 0;
	}
	if ((cl == 0x00 && ch == 0x00) || (cl == 0x3c && ch == 0xc3)) {
		if (!ata_device_init(dev)) return 0;
		int sectors = ata_max_offset(dev);
		if (sectors == 0) {
			// ATA must have at less 1 sector(512B)
			printf("ATA: device sectors are zero!\n");
			return 0;
		}
		printf("ATA: detected drive: %s, serial: %s\n",dev->identify.model,dev->identify.serial);
		ata_create_device(true);
	} else if ((cl == 0x14 && ch == 0xEB) || (cl == 0x69 && ch == 0x96)) {
		printf("ATA: cdrom found\n");
		ata_create_device(false);
	} else {
		printf("ATA: unknown cl and ch return values %x %x\n",cl,ch);
	}
	return 0;
}
void ata_vdev_read(struct vfs_node *node,uint32_t offset,uint32_t how,void *buf) {
	printf("ATA: read didn't supported!\n");
}
void ata_vdev_write(struct vfs_node *node,uint32_t offset,uint32_t how,void *buf) {
	printf("ATA: write didn't supported\n");
}
void ata_create_device(bool hda) {
	char *name;
	if (hda) {
		name = "hd ";
		name[2] = ata_start_char;
		ata_start_char++;
	} else {
		name = "cd ";
		name[2] = ata_cdrom_char;
		ata_cdrom_char++;
	}
#ifdef DEBUG
	write_serialString("creating ATA device with name: ");
	write_serialString(name);
	write_serialString("\r\n");
#endif
	dev_t *disk = pmml_alloc(true);
	disk->name = name;
	disk->write = ata_vdev_write;
	disk->read = ata_vdev_read;
	disk->buffer_sizeMax = 1; // didn't supportes so 1
	// Now register device in our DEVFS
	dev_add(disk);
}
// === Public functions here ===
void atapi_init() {
	printf("ATA device driver\n");
	ata_device_detect(&ata_primary_master);
	ata_device_detect(&ata_primary_slave);
	ata_device_detect(&ata_secondary_master);
	ata_device_detect(&ata_secondary_slave);
}
// === Loading as module support! ===
void module_main() {
	atapi_init();
}
