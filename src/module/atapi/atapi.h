#ifndef ATAPI_H
#define ATAPI_H
#include<typedefs.h>
void atapi_init();
void atapi_identify(bool master,int port);
#endif