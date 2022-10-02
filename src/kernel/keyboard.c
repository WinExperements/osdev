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
bool hasKey = false;
bool keyCode = 0;
void keyboard_handler(registers_t *regs);
void keyboard_keyHandler(char key);
void keyboard_init() {
    interrupts_addHandler(33,keyboard_handler);
	unsigned char kbb = 0;
   	while(((kbb = io_readPort(0x64)) & 1) == 1)
   	{
      		io_readPort(0x60);
   	}
	while(io_readPort(0x64) & 0x1) {
		io_readPort(0x60);
	}
	io_writePort(0x64,0xae);
	io_writePort(0x64,0x20);
	uint8_t status = (io_readPort(0x60) | 1)  & ~0x10;
	io_writePort(0x64,0x60);
	io_writePort(0x60,status);
	io_writePort(0x60,0xf4);
}
void keyboard_handler(registers_t *regs) {
	uint8_t key = io_readPort(0x60);
	if (key < 0x80)
	{
		switch(key)
        {
            case 0x02: keyboard_keyHandler('1'); break;
            case 0x03: keyboard_keyHandler('2'); break;
            case 0x04: keyboard_keyHandler('3'); break;
            case 0x05: keyboard_keyHandler('4'); break;
            case 0x06: keyboard_keyHandler('5'); break;
            case 0x07: keyboard_keyHandler('6'); break;
            case 0x08: keyboard_keyHandler('7'); break;
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
            case 0x2E: keyboard_keyHandler('c'); break;
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
                terminal_writeXY(' ',terminal_getX()-1,terminal_getY());
                keyboard_keyHandler('\b');
            } break;
            case 0x38: {
            } break;
            case 0x1d: {
            } break;
            case 0x28: keyboard_keyHandler('\''); break;
            default:
            {
                printf("Unhandled key: %x\n",key);
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
