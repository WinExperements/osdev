/** Non-architecture specific methods are live here **/
#include<terminal.h>
#include<dev.h>
#include<keyboard.h>
#include<stdarg.h>
#include<mm/pmm.h>
#include<arch.h>
#include<vfs.h>
dev_t *tty;
void printf_syscall(const char *msg) {
	int res;
	int num = 1;
	asm volatile("int $0x80" : "=a" (res) : "0" (num), "b" ((int)msg));
}
void printf(char *format,...) {
	va_list arg;
	va_start(arg,format);
	while (*format != '\0') {
		if (*format == '%') {
			if (*(format +1) == '%') {
				terminal_writestring("%");
			} else if (*(format + 1) == 's') {
				terminal_writestring(va_arg(arg,char*));
			} else if (*(format + 1) == 'c') {
				putc(va_arg(arg,int),0,0);
			} else if (*(format + 1) == 'd') {
				terminal_writeInt(va_arg(arg,int));
			} else if (*(format + 1) == 'x') {
				terminal_writeHex(va_arg(arg,int));
			} else if (*(format + 1) == '\0') {
				PANIC("printf: next char is null!");
			} else {
				PANIC("Unknown %");
			}
			format++;
		} else {
			putc(*format,0,0);
		}
		format++;
	}
	va_end(arg);
}
void tty_write(void *buffer,int size) {
  printf(buffer);
}
void tty_read(char *buff,int how) {
  int i = 0;
   while(i < (how-1)) {
    char c = keyboard_get();
    if (c == '\n') {
        buff[i] = 0;
        return;
    } else if (c == '\b') {
        if (i > 0) {
            i--;
        }
    } else if (c >= 0x20 && c <= 0x7e) {
        buff[i] = c;
        i++;
    }
   }
}
void tty_init() {
  tty = pmml_alloc(true);
  tty->name = "tty";
  tty->write = tty_write;
  tty->read = tty_read;
  tty->buffer_sizeMax = 1;
  dev_add(tty);
}
void terminal_writestring(const char *c) {
	int i =0;
	while(c[i]) {
		putc(c[i],0,0);
		i++;
    }
}
void terminal_writeInt(int u) {
	if (u == 0) {
          terminal_writestring("0");
          return;
        }
        s32 acc = u;
        char c[32];
        int i = 0;
        while(acc > 0) {
                c[i] = '0' + acc%10;
                acc /= 10;
                i++;
        }
        c[i] = 0;
        char c2[32];
        c2[i--] = 0;
        int j = 0;
        while(i >= 0) {
                c2[i--] = c[j++];
        }
	terminal_writestring(c2);
}
void terminal_writeHex(int num) {
	uint32_t tmp;
	terminal_writestring("0x");
	char noZeroes = 1;
	int i;
	for (i = 28; i > 0; i-=4) {
		tmp = (num >> i) & 0xF;
		if (tmp == 0 && noZeroes != 0) continue;
		if (tmp >= 0xA) {
			noZeroes = 0;
			putc(tmp-0xA+'a',0,0);
		} else {
			noZeroes = 0;
			putc(tmp+'0',0,0);
		}
	}
	tmp = num & 0xF;
	if (tmp >= 0xA) {
		putc(tmp-0xA+'a',0,0);
	} else {
		putc(tmp+'0',0,0);
	}
}