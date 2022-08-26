#include <terminal.h>
#include<io.h>
#include<atapi/atapi.h>
#include<serial.h>
#include<dev.h>
#include<mm/pmm.h>
void atapi_init() {
    printf("ATAPI driver for HelinOS kernel\n");
    printf("Detecting master drive...");
    atapi_identify(true,0x1f0);
    printf("Detecting master slave drive...");
    atapi_identify(false,0x1f0);
    printf("Detecting secondary master...");
    atapi_identify(true,0x107);
    printf("Detecting secondary slave...");
    atapi_identify(false,0x170);
}
void atapi_identify(bool master,int port) {
    int dataPort = port;
    int sectorCountPort = port+0x2;
    int lbaLowPart = port+0x3;
    int lbaMidPort = port+0x4;
    int lbaHiPort = port+0x5;
    int devicePort = port+0x6;
    int commandPort = port+0x7;
    int controlPort = port+0x206;
    io_writePort(devicePort,master ? 0xA0 : 0xB0);
    io_writePort(controlPort,0);
    io_writePort(devicePort,0xa0);
    uint8_t status = io_readPort(commandPort);
    if (status == 0xff) {
        goto notFound;
    }
    io_writePort(devicePort,master ? 0xA0 : 0xB0);
    io_writePort(sectorCountPort,0);
    io_writePort(lbaLowPart,0);
    io_writePort(lbaMidPort,0);
    io_writePort(lbaHiPort,0);
    io_writePort(commandPort,0xEC);
    status = io_readPort(commandPort);
    if (status == 0) {
        goto notFound;
    }
    while(((status & 0x80) == 0x80) && ((status & 0x1) != 0x1)) {
        status = io_readPort(commandPort);
    }
    if (status & 0x1) {
        printf("ATA detection error!\n");
        goto notFound;
    }
    printf("ok\n");
    /*dev_t *t = pmml_alloc(true);
    t->name = "sda";
    t->buffer_sizeMax = 2;*/
    //dev_add(t);
    for (int i = 0; i < 256; i++) {
        if (i < 40) return;
        printf("%c",io_readPortW(dataPort));;
    }
    return;
    notFound:
    printf("fail\n");
    return;
}