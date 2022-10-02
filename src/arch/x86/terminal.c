#include<terminal.h>
#include<io.h>
#include<serial.h>
#include<stdarg.h>
#include<arch.h>
#include<dev.h>
#include<mm/pmm.h>
#include<keyboard.h>
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
  // disable cursor
  io_writePort(0x3d4,0x0a);
  io_writePort(0x3d5,0x20);
  #endif
	psf_init();
	terminal_clear();
}
void scroll() {
  if (cursor_y >= ws_row) {
    #ifdef LEGACY_TERMINAL
    for (int i = 0*80; i < 24*80; ++i) {
      video_memory[i] = video_memory[i+80];
    }
    for (int i = 24*80; i < 25*80; ++i) {
      video_memory[i] = 0x20 | (((vgaBack << 4) | (vgaFr & 0x0f)) << 8);
    }
    cursor_y = 24;
    #else
    terminal_clear(); // we don't support it
    cursor_y = cursor_x = 0;
    #endif
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
  if (replay) {
      write_serial(ch);
  }
  scroll();
  terminal_printCursor(cursor_x,cursor_y);
}
void terminal_clear() {
  #ifndef LEGACY_TERMINAL 
	if (vgaW != 0) {
		for (uint32_t y = 0; y < vgaH; ++y) {
      for (uint32_t x = 0; x < vgaW; ++x) {
        vgaFramebuffer[x+y*vgaW] = vgaBack;
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
void terminal_setcolor(uint32_t colo) {
       vgaFr = colo;
}
void terminal_setbackground(uint32_t back) {
  vgaBack = back;
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
	uint8_t attributeByte = (bg << 4) | (fg & 0x0f);
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
  putchar(' ',x,y,vgaBack,vgaFr);
}
void terminal_writeXY(char c,uint16_t x,uint16_t y) {
  putchar(' ',cursor_x,cursor_y,vgaFr,vgaBack);
  cursor_x = x;
  cursor_y = y;
  putchar(c,x,y,vgaFr,vgaBack);
  terminal_printCursor(x,y);
}
int terminal_getBufferSize() {
  #ifndef LEGACY_TERMINAL
  int size = vgaP*vgaH;
  #else
  int size = 80*20;
  #endif
  return size;
}
