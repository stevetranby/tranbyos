#include <system.h>
#include <multiboot.h>

/* You will need to code these up yourself!  */
byte *memcpy(byte *dest, const byte *src, size_t count)
{
const byte *sp = (const byte *)src;
byte *dp = (byte *)dest;
for(; count != 0; count--) *dp++ = *sp++;
return dest;
}

byte *memset(byte *dest, byte val, size_t count)
{
/* Add code here to set 'count' bytes in 'dest' to 'val'.
*  Again, return 'dest' */

byte *temp = dest;
for( ; count != 0; count--) *temp++ = val;
return dest;
}

uint16 *memsetw(uint16 *dest, uint16 val, size_t count)
{
/* Same as above, but this time, we're working with a 16-bit
*  'val' and dest pointer. Your code can be an exact copy of
*  the above, provided that your local variables if any, are
*  uint16 */
uint16 *temp = (uint16 *)dest;
for( ; count != 0; count--) *temp++ = val;
return dest;
}

int strlen(const uint8 *str)
{
/* This loops through character array 'str', returning how
*  many characters it needs to check before it finds a 0.
*  In simple words, it returns the length in bytes of a string */
size_t retval;
for(retval = 0; *str != '\0'; str++) retval++;
return retval;
}

/* We will use this later on for reading from the I/O ports to get data
*  from devices such as the keyboard. We are using what is called
*  'inline assembly' in these routines to actually do the work */
uint8 inportb (uint16 _port)
{
uint8 rv;
__asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (_port));
return rv;
}

/* We will use this to write to I/O ports to send bytes to devices. This
*  will be used in the next tutorial for changing the textmode cursor
*  position. Again, we use some inline assembly for the stuff that simply
*  cannot be done in C */
void outportb (uint16 _port, uint8 _data)
{
__asm__ __volatile__ ("outb %1, %0" : : "dN" (_port), "a" (_data));
}

/* This is a very simple main() function. All it does is sit in an
*  infinite loop. This will be like our 'idle' loop */
int _main(multiboot_info_t* mbd, uint32 magic)
{
	gdt_install();
	idt_install();
	isrs_install();
	irq_install();
	
	init_mm();
	
	init_video();
	timer_install();
	keyboard_install();
	
	__asm__ __volatile__ ("sti");
	
	puts("\n");
	puts("Tranix: Tran[byU]nix\n"); 
	puts("By Steve Tranby\n");
	puts("\n");
	settextcolor(0x02, 0x00);
	puts("With Help from:");
	puts("    http://www.osdev.org/\n");
	puts("    http://www.osdever.net/bkerndev/index.php\n");
	puts("\n");
	settextcolor(0x03, 0x00);
	puts("This operating system is a test bed for me to \n");
	puts("experiment with writing an operating system kernel\n");
	puts("and possibly more\n");
	puts("\n");
	puts("\n");

	int i;
	int * p = 0;
	puts("The address of p is: ");
	printInt((int)&p);
	putch('\n');
	// DEBUG::Testing heap code
	puts("Heap Magic: ");
	print_heap_magic();
	putch('\n');
	
	byte *t = kmalloc(4);
	byte *s = kmalloc(8);
	for(i=0; i<4; ++i) {
		t[i] = i;
		s[i*2] = i*2;
		s[i*2+1] = i*2+1;
	}
	putch('\n');
	for(i=0; i<4; ++i) {
		printInt((int)t);
		putch(':');
		printInt((int)t[i]);
		putch(':');
		putch(':');
		printInt((int)s);
		putch(':');
		printInt((int)s[i*2]);
		putch(':');
		printInt((int)s[i*2+1]);
		putch('\n');
	}
	
	puts("\nSize of RAM(Lower) = ");
	printInt((int)mbd->mem_lower * 1024);
	puts("\nSize of RAM(Upper) = ");
	printInt((int)mbd->mem_upper * 1024);
	// print out some memory
	putch('\n');
	int array[10] = { 1,1,2,3,5,8,13,21,34 };
	p = array;
	for(i=0; i<10; ++i, ++p) {
		printInt((int)&p);
		putch(':');
		printInt((int)*p);
		putch('\t');
	}
	
	// Test Division By 0
	// i = 10 / 0; putch(i);
	
	
	/* ...and leave this loop in. There is an endless loop in
	*  'start.asm' also, if you accidentally delete this next line */
	settextcolor(0x0f, 0x00);
	puts("Starting Infinite Loop!\n");
	for (;;);
	return 0;
}

