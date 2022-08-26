#include "stdlib.h"
extern int main();
void _start() {
	exit(main());
	// something went wrong
	while(1) {}
}
