#include "include/system.h"

/*
 * These define our textpointer, our background and foreground
 * colors (attributes), and x and y cursor coordinates
 */
u16* textmemptr;		// word pointer
u16  attrib = 0x0F;		// attribute for text colors
u16  csr_x = 0;
u16  csr_y = 0;	// since these shouldn't ever be neg.

char palette16[16];
char palette256[768];

u16* vidmemptr;        // word pointer

bool write_to_serial = false;
bool write_to_stdout = false;

/* Sets the forecolor and backcolor that we will use */
void set_text_color(u8 forecolor, u8 backcolor)
{
    /* Top 4 bytes are the background, bottom 4 bytes
     *  are the foreground color */
    attrib = (backcolor << 4) | (forecolor & 0x0F);
}

/* Sets our text-mode VGA pointer, then clears the screen for us */
void init_video()
{
    textmemptr = (u16*)0xB8000;
    vidmemptr = (u16*)0xA0000;
    cls();
}

/* Scrolls the screen */
void scroll(void)
{
    u16 blank;
    u32 temp;

    /* A blank is defined as a space... we need to give it
     *  backcolor too */
    blank = 0x20 | (attrib << 8);

    /* Row 25 is the end, this means we need to scroll up */
    if(csr_y >= 25)
    {
        /* Move the current text chunk that makes up the screen
         *  back in the buffer by a line */
        temp = csr_y - 25 + 1;
        memcpy ((u8*)textmemptr, (u8*)(textmemptr + temp * 80), (25 - temp) * 80 * 2);

        /* Finally, we set the chunk of memory that occupies
         *  the last line of text to our 'blank' character */
        memsetw ((u16*)(textmemptr + (25 - temp) * 80), blank, 80);
        csr_y = 25 - 1;
    }
}

/* Updates the hardware cursor: the little blinking line
 *  on the screen under the last character pressed! */
void move_csr(void)
{
    u32 temp;

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
    u16 blank;
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

// TODO: use STDIN or terminal input buffer, read next byte if exists, else wait
u8 getch()
{
    u8 ch = keyboard_read_next();
    while(0 == ch)
    {
        delay_ms(10);
//        puts("getch: next = ");
//        putch(ch);
//        putch('\n');
        ch = keyboard_read_next();
    }
    return ch;
}

// TODO: should store into a buffer instead
/* Puts a single character on the screen */
void putch(u8 c)
{
    u16 *where;
    u16 att = attrib << 8;

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
void puts(const char* text)
{
    while(*text != 0) {
        putch(*text++);
    }
}

void printInt(i32 l) { writeInt(l, putch); }
void printHex(u32 l) { writeHex(l, putch); }
void printHex_w(u16 w) { writeHex_w(w, putch); }
void printHex_b(u8 b) { writeHex_b(b, putch); }
void printAddr(void* addr) { writeAddr(addr, putch); }
void printBinary_b(u8 num) { writeBinary_b(num, putch); }

#define MAX_INT_DIGITS 20

// TODO: should allow wrap on word
// TODO: should allow padding (at least with (0) zeros)
// Note: assume base 10 for right now
void writeInt(i32 num, output_writer writer)
{
    bool isNeg = false;
    u8 buf[MAX_INT_DIGITS];
    i32 cur, end, temp=0;

    end = MAX_INT_DIGITS-1;
    cur = end;

    if(num == 0) {
        writer('0');
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
        writer('-');

    // print the string
    char c = '0';
    for(; cur < end; ++cur) {
        c = buf[cur];
        writer(c);
    }
}

void writeUInt(u32 num, output_writer writer)
{
    bool isNeg = false;
    u8 buf[MAX_INT_DIGITS];
    i32 cur, end, temp=0;

    end = MAX_INT_DIGITS-1;
    cur = end;

    if(num == 0) {
        writer('0');
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
        writer('-');

    // print the string
    char c = '0';
    for(; cur < end; ++cur) {
        c = buf[cur];
        writer(c);
    }
}

void writeAddr(void* ptr, output_writer writer)
{
    writeHex((u32)ptr, writer);
}

void writeHex_b(u8 b, output_writer writer)
{
    puts("0x");
    writeHexDigit((b >> 4) & 0x0f, writer);
    writeHexDigit((b) & 0x0f, writer);
}

void writeHex_w(u16 w, output_writer writer)
{
    puts("0x");
    writeHexDigit((w>>12) & 0x0f, writer);
    writeHexDigit((w>>8) & 0x0f, writer);
    writeHexDigit((w>>4) & 0x0f, writer);
    writeHexDigit((w) & 0x0f, writer);
}

void writeHex(u32 w, output_writer writer)
{
    puts("0x");
    writeHexDigit((w>>28) & 0x0f, writer);
    writeHexDigit((w>>24) & 0x0f, writer);
    writeHexDigit((w>>20) & 0x0f, writer);
    writeHexDigit((w>>16) & 0x0f, writer);
    writeHexDigit((w>>12) & 0x0f, writer);
    writeHexDigit((w>>8) & 0x0f, writer);
    writeHexDigit((w>>4) & 0x0f, writer);
    writeHexDigit((w) & 0x0f, writer);
}

void writeHexDigit(u8 digit, output_writer writer)
{
    if(digit < 10)
        writeInt(digit, writer);
    else {
        switch(digit) {
            case 10: writer('a'); break;
            case 11: writer('b'); break;
            case 12: writer('c'); break;
            case 13: writer('d'); break;
            case 14: writer('e'); break;
            case 15: writer('f'); break;
        }
    }
}

void writeBinary_b(u8 num, output_writer writer)
{
    int i = 8;
    while(i--) {
        writeInt((num & (1<<i)) >> i, writer);
    }
}

void writeBinary_w(u16 num, output_writer writer)
{
    int i = 16;
    while(i--) {
        writeInt((num & (1<<i)) >> i, writer);
    }
}

void writeBinary(u32 num, output_writer writer)
{
    int i = 32;
    while(i--) {
        writeInt((num & (1<<i)) >> i, writer);
    }
}

//////////////////////////////////////////////////////////////////
// Serial Port Communication - mostly used for debug w/QEMU

#define PORT_COM1 0x3f8   /* COM1 */

void init_serial()
{
    outb(PORT_COM1 + 1, 0x00);    // Disable all interrupts
    outb(PORT_COM1 + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    outb(PORT_COM1 + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
    outb(PORT_COM1 + 1, 0x00);    //                  (hi byte)
    outb(PORT_COM1 + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(PORT_COM1 + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
    outb(PORT_COM1 + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

int serial_received() {
    return inb(PORT_COM1 + 5) & 1;
}

char read_serial() {
    while (serial_received() == 0);

    return inb(PORT_COM1);
}

u32 is_transmit_empty() {
    return inb(PORT_COM1 + 5) & 0x20;
}

void serial_write_b(u8 a) {
    while (is_transmit_empty() == 0)
        ;
    outb(PORT_COM1,a);
}

void serial_writeln(c_str str) {
    while (*str != 0) {
        serial_write_b(*str);
        str++;
    }
    serial_write_b('\r');
    serial_write_b('\n');
}

void serial_write(c_str str) {
    while (*str != 0) {
        if(*str == '\n') {
            serial_write_b('\r');
            serial_write_b('\n');
        }
        serial_write_b(*str);
        str++;
    }
}

void serial_writeInt(u32 number)
{
    writeInt(number, serial_write_b);
}

void serial_writeHex(u32 number)
{
    writeHex(number, serial_write_b);
}
