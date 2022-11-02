/* hello module, an internal test for module loading */
#include<terminal.h>
// define our module name in .modname section that needed for our module loader!
__attribute__((section(".modname"))) char *name = "hello";
void module_main() {
    printf("Hellllloooo\n");
}
