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
uint8 inb (uint16 _port)
{
uint8 rv;
__asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (_port));
return rv;
}

/* We will use this to write to I/O ports to send bytes to devices. This
*  will be used in the next tutorial for changing the textmode cursor
*  position. Again, we use some inline assembly for the stuff that simply
*  cannot be done in C */
void outb (uint16 _port, uint8 _data)
{
__asm__ __volatile__ ("outb %1, %0" : : "dN" (_port), "a" (_data));
}

uint8 inportb(uint16 _port) {
	return inb(_port);
}
void outportb(uint16 _port, uint8 _data) {
	outb (_port,_data);
}

/* */
int _main(multiboot_info_t* mbd, uint32 magic)
{
	int i;

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
	puts("Tranbonix OS\n"); 
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

	// -- BEG HARD DISK ACCESS TESTING ---

	print_hd_device_types();
	putch('\n');

/*
	// 0x1F7 : Read
	// bit 7 = 1  controller is executing a command
	// bit 6 = 1  drive is ready
	// bit 5 = 1  write fault
	// bit 4 = 1  seek complete
	// bit 3 = 1  sector buffer requires servicing
	// bit 2 = 1  disk data read corrected
	// bit 1 = 1  index - set to 1 each revolution
	// bit 0 = 1  previous command ended in an error

	// Testing Hard Disk Access and Info Gathering

	outportb(0x1F6, 0xA0); // drive and head port, drive 0, head 0 (101dhhhh - d:drive, h:head)
	outportb(0x1F2, 0x01); // sector count port, read 1 sector
	outportb(0x1F3, 0x01); // sector # port, read sector 1
	outportb(0x1F4, 0x00); // cyl low port, cyl 0
	outportb(0x1F5, 0x00); // cyl high port, rest of cyl 0
	outportb(0x1F7, 0x30); // cmd port, read with retry
	
	for(i=0; i<2; ++i) {
		uint8 status = inportb(0x1F7);
	
		uint8 status_execmd = status & 0x80; // 1000 0000
		uint8 status_drvrdy = status & 0x40; // 0100 0000
		uint8 status_wrtflt = status & 0x20; // 0010 0000
		uint8 status_skcmpl = status & 0x10; // 0001 0000
		uint8 status_sbrqsv = status & 0x08; // 0000 1000
		uint8 status_ddrcor = status & 0x04; // 0000 0100
		uint8 status_idxrev = status & 0x02; // 0000 0010
		uint8 status_cmderr = status & 0x01; // 0000 0001
	
		printInt(status_execmd >> 7);
		printInt(status_drvrdy >> 6);
		printInt(status_wrtflt >> 5);
		printInt(status_skcmpl >> 4);
		printInt(status_sbrqsv >> 3);
		printInt(status_ddrcor >> 2);
		printInt(status_idxrev >> 1);
		printInt(status_cmderr); 
		putch('\n');
	}

	// Don't continue until sector buffer is ready (not executing)
	//while(inportb(0x1F7) & 0x80) {}

	// should check for any errors
*/

/*
   // read the bytes of this sector
	byte data;
	for(i=0; i<512; ++i) {
		data = inportb(0x1F0);
		printHex(data); putch(',');
	}
*/

//	putch('\n');
//	putch('\n');

	// 

	// -- END HARD DISK ACCESS TESTING ---

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

