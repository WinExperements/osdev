/** non architecture voids */
#include<typedefs.h>
#include<terminal.h>
#include<arch.h>
void kassert(const char *file,const char *func,int line,bool exc) {
    if (!exc) {
        printf("Assert in %s:%s:%d\n",file,func,line);
        PANIC("Assertion.");
    }
}