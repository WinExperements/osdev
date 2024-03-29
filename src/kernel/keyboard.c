#include <keyboard.h>
#include <arch.h>
#include <io.h>
#include <interrupts.h>
#include <x86/idt.h>
#include <terminal.h>
#include<x86/pic.h>
#include<mm/pmm.h>
#include<mm/vmm.h>
#include <serial.h>
#include<process.h>
bool hasKey = false;
int keyCode = 0;
bool shift,caps,ctrl = false;
void keyboard_handler(registers_t *regs);
void keyboard_keyHandler(char key);
void keyboard_init() {
	io_writePort(0x64,0xAD); // disable first PS/2 port
	io_writePort(0x64,0xA7); // disable second PS/2 port(if supported)
	while(io_readPort(0x64) & 0) {
	}
	// re-enable disabled ports
	io_writePort(0x64,0xAE);
	io_writePort(0x64,0xA8);
	interrupts_addHandler(33,keyboard_handler);
	// register second PS/2 port interrupt handler
	interrupts_addHandler(IRQ12,keyboard_handler);
}
void keyboard_handler(registers_t *regs) {
	uint8_t key = io_readPort(0x60);
	if (key < 0x80)
	{
        //printf("key: %x\n",key);
		switch(key)
        {
            case 0x02: keyboard_keyHandler('1'); break;
            case 0x03: keyboard_keyHandler('2'); break;
            case 0x04: keyboard_keyHandler('3'); break;
            case 0x05: keyboard_keyHandler('4'); break;
            case 0x06: keyboard_keyHandler('5'); break;
            case 0x07: keyboard_keyHandler('6'); break;
            case 0x08: {
                if (!shift) {
                    keyboard_keyHandler('7');
                } else {
                    keyboard_keyHandler('&');
                }
            } break;
            case 0x09: keyboard_keyHandler('8'); break;
            case 0x0A: keyboard_keyHandler('9'); break;
            case 0x0B: keyboard_keyHandler('0'); break;

            case 0x10: keyboard_keyHandler('q'); break;
            case 0x11: keyboard_keyHandler('w'); break;
            case 0x12: keyboard_keyHandler('e'); break;
            case 0x13: keyboard_keyHandler('r'); break;
            case 0x14: keyboard_keyHandler('t'); break;
            case 0x15: keyboard_keyHandler('y'); break;
            case 0x16: keyboard_keyHandler('u'); break;
            case 0x17: keyboard_keyHandler('i'); break;
            case 0x18: keyboard_keyHandler('o'); break;
            case 0x19: keyboard_keyHandler('p'); break;

            case 0x1E: keyboard_keyHandler('a'); break;
            case 0x1F: keyboard_keyHandler('s'); break;
            case 0x20: keyboard_keyHandler('d'); break;
            case 0x21: keyboard_keyHandler('f'); break;
            case 0x22: keyboard_keyHandler('g'); break;
            case 0x23: keyboard_keyHandler('h'); break;
            case 0x24: keyboard_keyHandler('j'); break;
            case 0x25: keyboard_keyHandler('k'); break;
            case 0x26: keyboard_keyHandler('l'); break;

            case 0x2C: keyboard_keyHandler('z'); break;
            case 0x2D: keyboard_keyHandler('x'); break;
            case 0x2E: {
                if (ctrl) {
                    printf("^C\n");
                    ctrl = !ctrl;
                    // kill current thread
                    process_kill(process_getCurrentPID());
                    arch_enableInterrupts();
                    process_yield();
                    break;
                }
                keyboard_keyHandler('c');
            } break;
            case 0x2F: keyboard_keyHandler('v'); break;
            case 0x30: keyboard_keyHandler('b'); break;
            case 0x31: keyboard_keyHandler('n'); break;
            case 0x32: keyboard_keyHandler('m'); break;
            case 0x33: keyboard_keyHandler(','); break;
            case 0x34: keyboard_keyHandler('.'); break;
            case 0x35: keyboard_keyHandler('/'); break;

            case 0x1C: keyboard_keyHandler('\n'); break;
            case 0x39: keyboard_keyHandler(' '); break;
            case 0xe: {
                keyboard_keyHandler('\b');
            } break;
            case 0x2b: keyboard_keyHandler('\\'); break;
            case 0x2a: {shift = !shift;} break;
            case 0x1d: {ctrl = !ctrl;} break;
            default:
            {
		printf("Unknown key: %x\n",key);
                break;
            }
	}
	}
}
char keyboard_get() {
	hasKey = false;
	// forever loop, if key pressed it's breaks
	while(!hasKey) {
    }
	return keyCode;
}
void keyboard_keyHandler(char key) {
    if (key != '\b') {
        printf("%c",key);
    }
    hasKey = true;
    keyCode = key;
}
