#include<stdint.h>
#define UART0_BASE 0x1c900000
void write_uart(char c) {
    *(volatile uint32_t *)(UART0_BASE) = c;
}
void write_string_uart(char *message) {
    while (*message != 0) {
        write_uart(*message++);
    }
}
void kernel_main() {
    // our entry point, currently
    write_string_uart("HelinOS ARM\r\n");
}
