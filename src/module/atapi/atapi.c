#include <terminal.h>
#include<io.h>
#include<atapi/atapi.h>
#include<serial.h>
#include<dev.h>
#include<mm/pmm.h>
#include<interrupts.h>
// === Internal functions here ===
typedef struct {
	uint16_t base;
	uint16_t ctrl;
	uint16_t nein;
} ata_regs_t;
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
	ata_regs_t *regs;
} ata_device_t;
void ata_io_wait(ata_device_t *dev) {
	io_readPort(dev->regs->base+0x0c);
	io_readPort(dev->regs->base+0x0c);
	io_readPort(dev->regs->base+0x0c);
	io_readPort(dev->regs->base+0x0c);
}
int ata_status_wait(ata_device_t *dev,int timeout) {
	int status;
	if (timeout > 0) {
		int i =0;
		while((status = io_readPort(dev->regs->base+0x07) & 0x0 && (i < timeout)) i++;
	} else {
		while((status = io_readPort(dev->regs->base+0x07)) & 0x0);
	}
	return status;	
}
// === Public functions here ===
void atapi_init() {
}
