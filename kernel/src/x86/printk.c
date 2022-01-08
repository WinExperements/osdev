#include "io.h"
#include "printk.h"
u16 *video_memory;
u8 cursor_x;
u8 cursor_y;
int initialized;
void printk_init() {
	cursor_x = cursor_y = 0;
	video_memory = (u16 *)0xB8000;
	initialized = 1;
}
void movecursor() {
	u16 pos = cursor_y * 80 + cursor_x;
	io_writePort(0x3D4,14);
	io_writePort(0x3D5,pos>>16);
	io_writePort(0x3D4,15);
	io_writePort(0x3D5,pos);
}
void scroll() {
	u8 attribute = ( BLACK << 4) | (WHITE & 0x0F);
  u8 blank = 0x20 | (attribute << 8);

  // A Standard Terminal size is 25 lines * 80 Coloumns
  if (cursor_y  >= 25)
    {
      // We are at the End of  the  Terminal so we have to move the entire text up one line
      int i;
      for(i = 0*80; i < 24*80; ++i)
        {
          video_memory[i] = video_memory[i+80];
        }

      // Now the Last Line is Left Blank , so we write 80 spaces using our blank character
      for (i = 24*80; i < 25*80; ++i)
        {
          video_memory[i] = blank;
        }

      // Set Y coordinate to last line
      cursor_y = 24;
    }
}
void putc(char ch) {
	u8 attribute =( BLACK << 4) |(WHITE & 0x0f);

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
      cursor_x = 0;
      cursor_y++;
    }

  // IF all the above text fails , print the Character
  if (ch >= ' ')
    {
      // Calculate the Address of the Cursor Position
      u16 *location = video_memory + (cursor_y * 80 + cursor_x);
      // Write the Bit into the cursor Postition
      *location = ch | (attribute << 8);
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
  movecursor();
}
void printk(char *msg) {
	int i = 0;
	while(msg[i]) {
		putc(msg[i++]);
	}
}
void printk_clear() {
	u8 attr = WHITE;
	u8 ch = 0x20;
	int i = 0;
  for (; i < 25*80; ++i)
    {
      video_memory[i] = ch | (attr << 8);
    }
    cursor_x = cursor_y = 0;
  movecursor();
}
void printkDec(int n) {
	if (n == 0)
    {
      putc('0');
      return;
    }

  s32 acc = n;
  char c[32];
  int i = 0;
  while (acc > 0)
    {
      c[i] = '0' + acc%10;
      acc /= 10;
      i++;
    }
  c[i] = 0;

  char c2[32];
  c2[i--] = 0;
  int j = 0;
  while(i >= 0)
    {
      c2[i--] = c[j++];
    }
    printk(c2);
}
