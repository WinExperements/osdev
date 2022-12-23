#include <terminal.h>
void module_main() {
	printf("Hello from hello.mod!\n");
	asm volatile("cli");
	for (;;) {}
}
