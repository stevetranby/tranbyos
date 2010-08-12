#include <system.h>

#define BLACK		0x0
#define BLUE		0x01	
#define GREEN		0x02
#define CYAN		0x03
#define RED		0x04
#define MAGENTA		0x05
#define BROWN		0x06	
#define LIGHT_GREY	0x07	
#define DARK_GREY	0x08	
#define LIGHT_BLUE	0x09
#define LIGHT_GREEN	0x10
#define LIGHT_CYAN	0x11
#define LIGHT_RED	0x12	
#define LIGHT_MAGENTA	0x13
#define LIGHT_BROWN	0x14
#define WHITE		0x15

/*
 * These define our textpointer, our background and foreground
 * colors (attributes), and x and y cursor coordinates 
 */
uint16 *textmemptr;		// word pointer
uint16 attrib = 0x0F;		// attribute for text colors
uint16 csr_x = 0, csr_y = 0;	// since these shouldn't ever be neg.

/* Scrolls the screen */
void scroll(void)
{
    uint16 blank;
    size_t temp;

    /* A blank is defined as a space... we need to give it
    *  backcolor too */
    blank = 0x20 | (attrib << 8);

    /* Row 25 is the end, this means we need to scroll up */
    if(csr_y >= 25)
    {
        /* Move the current text chunk that makes up the screen
        *  back in the buffer by a line */
        temp = csr_y - 25 + 1;
        memcpy ((byte *)textmemptr, (byte *)(textmemptr + temp * 80), (uint16) (25 - temp) * 80 * 2);

        /* Finally, we set the chunk of memory that occupies
        *  the last line of text to our 'blank' character */
        memsetw ((uint16*)(textmemptr + (25 - temp) * 80), blank, 80);
        csr_y = 25 - 1;
    }
}

/* Updates the hardware cursor: the little blinking line
*  on the screen under the last character pressed! */
void move_csr(void)
{
    size_t temp;

    /* The equation for finding the index in a linear
    *  chunk of memory can be represented by:
    *  Index = [(y * width) + x] */
    temp = csr_y * 80 + csr_x;

    /* This sends a command to indicies 14 and 15 in the
    *  CRT Control Register of the VGA controller. These
    *  are the high and low bytes of the index that show
    *  where the hardware cursor is to be 'blinking'. To
    *  learn more, you should look up some VGA specific
    *  programming documents. A great start to graphics:
    *  http://www.brackeen.com/home/vga */
    outb(0x3D4, 14);
    outb(0x3D5, temp >> 8);
    outb(0x3D4, 15);
    outb(0x3D5, temp);
}

/* Clears the screen */
void cls()
{
    uint16 blank;
    int i;

    /* Again, we need the 16-bits that will be used to
    *  represent a space with color */
    blank = 0x20 | (attrib << 8);

    /* Sets the entire screen to spaces in our current
    *  color */
    for(i = 0; i < 25; i++)
        memsetw (textmemptr + i * 80, blank, 80);

    /* Update our virtual cursor, and then move the
    *  hardware cursor */
    csr_x = 0;
    csr_y = 0;
    move_csr();
}

char getch() {
	return 0;
}

/* Puts a single character on the screen */
void putch(char c)
{
    uint16 *where;
    uint16 att = attrib << 8;

    /* Handle a backspace, by moving the cursor back one space */
    if(c == 0x08)
    {
        if(csr_x != 0) csr_x--;
    }
    /* Handles a tab by incrementing the cursor's x, but only
    *  to a point that will make it divisible by 8 */
    else if(c == 0x09)
    {
        csr_x = (csr_x + 8) & ~(8 - 1);
    }
    /* Handles a 'Carriage Return', which simply brings the
    *  cursor back to the margin */
    else if(c == '\r')
    {
        csr_x = 0;
    }
    /* We handle our newlines the way DOS and the BIOS do: we
    *  treat it as if a 'CR' was also there, so we bring the
    *  cursor to the margin and we increment the 'y' value */
    else if(c == '\n')
    {
        csr_x = 0;
        csr_y++;
    }
    /* Any character greater than and including a space, is a
    *  printable character. The equation for finding the index
    *  in a linear chunk of memory can be represented by:
    *  Index = [(y * width) + x] */
    else if(c >= ' ')
    {
        where = textmemptr + (csr_y * 80 + csr_x);
        *where = c | att;	/* Character AND attributes: color */
        csr_x++;
    }

    /* If the cursor has reached the edge of the screen's width, we
    *  insert a new line in there */
    if(csr_x >= 80)
    {
        csr_x = 0;
        csr_y++;
    }

    /* Scroll the screen if needed, and finally move the cursor */
    scroll();
    move_csr();
}

/* Uses the above routine to output a string... */
void puts(char *text)
{
    uint16 i;

    for (i = 0; i < strlen((byte *)text); i++)
    {
        putch((byte)text[i]);
    }
}

#define MAX_INT_DIGITS 20
// assume base 10 for right now
void printInt(int number) {
	int num = number;
    int isNeg = 0;
	char buf[MAX_INT_DIGITS];
	int cur, end, temp=0;
	
	end = MAX_INT_DIGITS-1;
	cur = end;
	
	if(num == 0) {
		putch('0');
		return;
	}
	
	buf[cur] = '\0';
	
	// check if negative and print neg character
	if(num < 0) {
		isNeg = 1;
		num = -num;		// abs(num)
	}
	
	while(num) {
		temp = num % 10; // get 'ones' or right most digit
		--cur;
		buf[cur] = temp + '0'; // add the digit (temp) to ASCII code '0'				
		num /= 10; // remove right most digit
	}
	
	if(isNeg) 
		putch('-');
		
	// print the string
	char c = '0';
	for(; cur < end; ++cur) {
		c = buf[cur];
		putch(c);
	}	
}

// print out the byte in hex 
void printHex(byte b) {		
	printHexDigit((b >> 4) & 0x0f);
	printHexDigit((b) & 0x0f);
}

void printHex_w(word w) {	
	printHexDigit((w>>12) & 0x0f);
	printHexDigit((w>>8) & 0x0f);
	printHexDigit((w>>4) & 0x0f);
	printHexDigit((w) & 0x0f);
}

void printHexDigit(byte digit) {
	if(digit < 10) 
		printInt(digit);
	else {
		switch(digit) {
			case 10: putch('a'); break;
			case 11: putch('b'); break;
			case 12: putch('c'); break;
			case 13: putch('d'); break;
			case 14: putch('e'); break;
			case 15: putch('f'); break;
		}
	}
}

void printBin_b(byte num) {
	int i = 8;
	while(i--) {
		printInt((num & (1<<i)) >> i);		
	}	
}

void printBin_w(word num) {
	int i = 16;
	while(i--) {	
		printInt((num & (1<<i)) >> i);		
	}
}

/* Sets the forecolor and backcolor that we will use */
void settextcolor(byte forecolor, byte backcolor)
{
    /* Top 4 bytes are the background, bottom 4 bytes
    *  are the foreground color */
    attrib = (backcolor << 4) | (forecolor & 0x0F);
}

/* Sets our text-mode VGA pointer, then clears the screen for us */
void init_video(void)
{
    textmemptr = (uint16 *)0xB8000;
    cls();
}



