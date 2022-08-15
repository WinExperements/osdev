#include<terminal.h>
#include<io.h>
#include<serial.h>
#include<stdarg.h>
#include<arch.h>
#include<dev.h>
#include<mm/pmm.h>
dev_t *tty;
uint16_t cursor_x;
uint16_t cursor_y;
int initialized;
uint32_t *vgaFramebuffer;
uint32_t vgaW,vgaH,vgaP;
extern char _binary_font_psf_start;
extern char _binary_font_psf_end;
uint16_t *unicode;
bool replay;
void terminal_clear();
void psf_init();
void putchar(unsigned short int c,int,int,uint32_t,uint32_t);
uint32_t vgaBack,vgaFr,vgaBpp;
uint32_t ws_row,ws_col;
uint16_t *video_memory;
void terminal_initialize(struct multiboot_info *info) {
	cursor_x = cursor_y = 0;
  	vgaBack = BLACK;
  	vgaFr = WHITE;
	video_memory = (uint16_t *)0xB8000;
	// init VGA
	vgaFramebuffer = (uint32_t *)info->framebuffer_addr;
	vgaW = info->framebuffer_width;
	vgaH = info->framebuffer_height;
	vgaP = info->framebuffer_pitch;
  vgaBpp = info->framebuffer_bpp;
  #ifndef LEGACY_TERMINAL
	ws_row = vgaH/16;
 	ws_col = vgaW/9;
  #else
  ws_row = 25;
  ws_col = 80;
  #endif
	psf_init();
	terminal_clear();
}
void movecursor(uint16_t x,uint16_t y) {
	uint16_t pos = y * 80 + x;
	io_writePort(0x3D4,14);
	io_writePort(0x3D5,pos>>16);
	io_writePort(0x3D4,15);
	io_writePort(0x3D5,pos);
}
void scroll() {
  if (cursor_y >= ws_row) {
    terminal_clear();
    cursor_x = cursor_y = 0;
    #ifndef LEGACY_TERMINAL
    putchar(' ',0,0,vgaBack,vgaFr);
    #endif
  }
}
void putc(char ch,uint32_t back,uint32_t color) {
	bool writeCursor = true;

  // Handle a backspace, by moving the cursor back one space
  if (ch == 0x08 && cursor_x)
    {
      cursor_x--;
    }

  // Handle a tab by increasing the cursor's X, but only to a point
  // where it is divisible by 8.
  else if (ch == 0x09)
    {
      cursor_x = (cursor_x+8) & ~(8-1);
    }

  // Handle carriage return
  else if (ch == '\r')
    {
      cursor_x = 0;
    }

  // Handle newline by moving cursor back to left and increasing the row
  else if (ch == '\n')
    {
      putchar(' ',cursor_x,cursor_y,vgaFr,vgaBack);
      cursor_x = 0;
      cursor_y++;
    }

  // IF all the above text fails , print the Character
  if (ch >= ' ')
    {
      // Calculate the Address of the Cursor Position
      putchar(ch,cursor_x,cursor_y,vgaFr,vgaBack);
      cursor_x++;
    }

  // IF after all the printing we need to insert a new line
  if (cursor_x >= 80)
    {
      cursor_x = 0;
      cursor_y++;
    }

  // Scroll , or move the Cursor If Needed
  scroll();
  terminal_printCursor(cursor_x,cursor_y);
}
void terminal_writestring(const char *c) {
	int i =0;
	while(c[i]) {
		putc(c[i],0,0);
		i++;
	}
	//write_serialString(c);
}
void terminal_clear() {
  #ifndef LEGACY_TERMINAL 
	if (vgaW != 0) {
		for (uint32_t y = 0; y < vgaH; ++y) {
      for (uint32_t x = 0; x < vgaW; ++x) {
        vgaFramebuffer[x+y*vgaBpp] = vgaBack;
      }
    }
	}
  #else
  for (int i = 0; i < 80*25; i++) {
			video_memory[i] = 0x20 | (15 << 8);
		}
  #endif
  cursor_x = cursor_y = 0;
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
void terminal_setcolor(uint32_t colo) {
       vgaFr = colo;
}
void terminal_setbackground(uint32_t back) {
  vgaBack = back;
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
int terminal_getX() {
	return cursor_x;
}
int terminal_getY() {
	return cursor_y;
}
int terminal_getBufferAddress() {
	return (int)vgaFramebuffer;
}
void printf_syscall(const char *msg) {
	int res;
	int num = 1;
	asm volatile("int $0x80" : "=a" (res) : "0" (num), "b" ((int)msg));
}
void terminal_enableReplay(bool enable) {
	// Does we need to enable the replay to serial(Debug)
	replay = enable;
}
void putchar(
    /* note that this is int, not char as it's a unicode character */
    unsigned short int c,
    /* cursor position on screen, in characters not in pixels */
    int cx, int cy,
    /* foreground and 0 0s, say 0xFFFFFF and 0x000000 */
    uint32_t fg, uint32_t bg)
{
    #ifndef LEGACY_TERMINAL
    /* cast the address to PSF header struct */
    PSF_font *font = (PSF_font*)&_binary_font_psf_start;
    /* we need to know how many bytes encode one row */
    int bytesperline=(font->width+7)/8;
    /* unicode translation */
    if(unicode != NULL) {
        c = unicode[c];
    }
    /* get the glyph for the character. If there's no
       glyph for a given character, we'll display the first glyph. */
    unsigned char *glyph =
     (unsigned char*)&_binary_font_psf_start +
     font->headersize +
     (c>0&&c<font->numglyph?c:0)*font->bytesperglyph;
    /* calculate the upper left corner on screen where we want to display.
       we only do this once, and adjust the offset later. This is faster. */
    int offs =
        (cy * font->height * vgaP) +
        (cx * (font->width+1) * 4);
    /* finally display pixels according to the bitmap */
    int x,y, line,mask;
    for(y=0;y<font->height;y++){
        /* save the starting position of the line */
        line=offs;
        mask=1<<(font->width-1);
        /* display a row */
        for(x=0;x<font->width;x++)
        {
            if (c == 0)
            {
                *((uint32_t*)((uint8_t*)vgaFramebuffer + line)) = bg;
            }
            else
            {
                *((uint32_t*)((uint8_t*)vgaFramebuffer + line)) = ((int)*glyph) & (mask) ? fg : bg;
            }

            /* adjust to the next pixel */
            mask >>= 1;
            line += 4;
        }
        /* adjust to the next line */
        glyph += bytesperline;
        offs  += vgaP;
    }
    #else
	/* print character into BIOS text buffer */
	uint8_t attributeByte = (0 << 4) | (15 & 0x0f);
	uint16_t attrib = attributeByte << 8;
	uint16_t *location = video_memory + (cy * 80 + cx);
	*location = c | attrib;
    #endif
}
void psf_init()
{
    uint16_t glyph = 0;
    /* cast the address to PSF header struct */
    PSF_font *font = (PSF_font*)&_binary_font_psf_start;
    /* is there a unicode table? */
    if (font->flags) {
        unicode = NULL;
        return; 
    }
	uint16_t _unicode[font->numglyph];
	unicode = &_unicode;
    /* get the offset of the table */
    char *s = (char *)(
    (unsigned char*)&_binary_font_psf_start +
      font->headersize +
      font->numglyph * font->bytesperglyph
    );
    /* allocate memory for translation table */
    while(s>_binary_font_psf_end) {
        uint16_t uc = (uint16_t)((unsigned char *)s[0]);
        if(uc == 0xFF) {
            glyph++;
            s++;
            continue;
        } else if(uc & 128) {
            /* UTF-8 to unicode */
            if((uc & 32) == 0 ) {
                uc = ((s[0] & 0x1F)<<6)+(s[1] & 0x3F);
                s++;
            } else
            if((uc & 16) == 0 ) {
                uc = ((((s[0] & 0xF)<<6)+(s[1] & 0x3F))<<6)+(s[2] & 0x3F);
                s+=2;
            } else
            if((uc & 8) == 0 ) {
                uc = ((((((s[0] & 0x7)<<6)+(s[1] & 0x3F))<<6)+(s[2] & 0x3F))<<6)+(s[3] & 0x3F);
                s+=3;
            } else
                uc = 0;
        }
        /* save translation */
        unicode[uc] = glyph;
        s++;
    }
}
void terminal_clearWithColor(uint32_t back) {
  vgaBack = back;
  terminal_clear();
}
void terminal_printCursor(uint16_t x,uint16_t y) {
#ifndef LEGACY_TERMINAL
  putchar(' ',x,y,vgaBack,vgaFr);
#else
  movecursor(x,y);
#endif
}
void terminal_writeXY(char c,uint16_t x,uint16_t y) {
  putchar(' ',cursor_x,cursor_y,vgaFr,vgaBack);
  cursor_x = x;
  cursor_y = y;
  putchar(c,x,y,vgaFr,vgaBack);
  terminal_printCursor(x,y);
}
int terminal_getBufferSize() {
  int size = vgaP*vgaH;
  return size;
}
void tty_write(void *buffer,int size) {
  printf(buffer);
}
void tty_init() {
  tty = pmml_alloc(true);
  tty->name = "tty";
  tty->write = tty_write;
  dev_add(tty);
}
