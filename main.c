#include <system.h>
#include <hd.h>
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
byte rv;
__asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (_port));
return rv;
}

word inw(word _port) {
	word rv;
	__asm__ volatile ("inw %1, %0" : "=a" (rv) : "dN" (_port));
	return rv;
}

dword ind(word _port) {
	dword rv;
	__asm__ volatile ("inl %1, %0" : "=a" (rv) : "dN" (_port));
	return rv;
}

/* We will use this to write to I/O ports to send bytes to devices. This
*  will be used in the next tutorial for changing the textmode cursor
*  position. Again, we use some inline assembly for the stuff that simply
*  cannot be done in C */
void outb (word _port, byte _data)
{
__asm__ __volatile__ ("outb %1, %0" : : "dN" (_port), "a" (_data));
}

void outw (word _port, word _data)
{
__asm__ __volatile__ ("outw %1, %0" : : "dN" (_port), "a" (_data));
}

void outd (word _port, dword _data)
{
__asm__ __volatile__ ("outl %1, %0" : : "dN" (_port), "a" (_data));
}

/* */
int _main(multiboot_info_t* mbd, uint32 magic)
{
	int i = 0;

	gdt_install();
	idt_install();
	isrs_install();
	irq_install();
	
	init_mm();
	
	init_video();
	timer_install();
	keyboard_install();
	
	__asm__ __volatile__ ("sti");
	
	/*
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
	*/
	
	//getch();

	// -- BEG HARD DISK ACCESS TESTING ---
	
	//reset_devices();
	
	// reset and set
	outb(0x3F6, 0x02|0x04);
	timer_wait(10);
	// wait while busy
	while((inb(0x1F7)&0x80));
	// setup command block registers 
	outb(0x1F2, 0x01);
	outb(0x1F3, 0x01);
	outb(0x1F4, 0x00);
	outb(0x1F5, 0x00);
	outb(0x1F6, 0xA0); // primary master, head 0
	// wait until ready
	while(!(inb(HD_ST)&HD_ST_RDY));	
	// write to command register
	outb(0x1F7, 0x30);	
	// wait until DRQ is set
	while(!(inb(HD_ST)&HD_ST_DRQ));
	
	
	
	// Detect if Controller is present
	/*
	outb(0x1f3, 0x8f);
	timer_wait(5);
	if(inb(0x1f3)==0x8f) {
		puts("Controller Ready\n");		
		
		// Detect if drive is present
		outb(0x1f6, 0xA0);
		timer_wait(5);
		if(inb(0xf7) & 0x40) {
			puts("primary master present\n");			
			puts("\nReading Data: ");
			hd_read_b(1, 512, 0);	
			putch('\n');
		} else {
			puts("primary master NOT present \n");
		}
		
	} else {
		puts("Controller Not Ready\n");
	}
	*/
	
	
/*
	print_hd_device_types();
	putch('\n');

	byte data[512];
	for(i=0; i<512; ++i) 
		data[i]=0xf0;
	data[0] = 0x05;
	data[1] = 0x10;
	data[2] = 0x05;
	data[3] = 0x15;
	data[4] = 0x10;
	data[5] = 0x05;		
*/	

	//puts("Writing Data: ");
	//hd_write_b(1, data, 512, 0);
	
	
	// -- Check HD w/BIOS
	
	
	/*
	
	word * tmp2;
	tmp2 = (word *) 0x1000;
	int hd_cylinders = (((tmp2[2]>>8) & 0xff) | ((tmp2[2]<<2) & 0x300)) +1;
	int hd_sectors = (tmp2[2] & 63);
	int hd_heads = ((tmp2[3]>>8) & 0xff) +1;
	
	int total_sectors = hd_cylinders * hd_heads * hd_sectors;	
	
	puts("BIOS Reported Geometry: \n");
	puts("Status : "); printHex_w(tmp2[0]); putch('\n');
	puts("Cyls : "); printInt(hd_cylinders); putch('\n');
	puts("Sectors : "); printInt(hd_sectors); putch('\n');
	puts("Heads : "); printInt(hd_heads); putch('\n');
	puts("Total Sectors: "); printInt(total_sectors); putch('\n');
	putch('('); printInt(total_sectors/2048); putch(')'); putch('\n');
	
	*/
	
	/*
	// -- Check HD w/Identify
	int status;
	
	if(reset_controller()) puts("Controller BUSY\n");
	if(!select_device(0)) puts("ERROR SELECT DEVICE\n");
	puts("controller 0: "); 
	printHex(inb(HD_ST));
	puts(" (error ");
	printHex(inb(HD_ERR));
	puts("), controller 1: ");	
	printHex(inb(0x177));
	putch('\n');
	
	outb(HD_CMD, HD_CMD_DIAGNOSE);
	puts("Todo: Waiting for IRQ\n");
	//while(irqe==0); irqe--;
	timer_wait(100);
	
	if(!select_device(0)) puts("ERROR SELECT DEVICE\n");
	outb(HD_CMD, 0x10);
	//while(irqe==0); irqe--;
	timer_wait(100);
	
	status = inb(HD_ST);
	status = inb(HD_DCR);
	puts("controller 0: "); 
	printHex(inb(HD_ST));
	puts(" (error ");
	printHex(inb(HD_ERR));
	puts("), controller 1: ");	
	printHex(inb(0x177));
	putch('\n');
	
	if(!select_device(0)) puts("ERROR SELECT DEVICE\n");
	puts("Waiting for controller\n");
	puts("controller 0: "); 
	printHex(inb(HD_ST));
	puts(" (error ");
	printHex(inb(HD_ERR));
	puts("), controller 1: ");	
	printHex(inb(0x177));
	putch('\n');
	while(!is_ready());
	
	outb(HD_CMD, HD_CMD_IDENTIFY);
	puts("Waiting for data\n");
	
	//while(irqe==0); irqe--;
	timer_wait(100);
	
	puts("Waiting for controller\n");
	puts("controller 0: "); 
	printHex(inb(HD_ST));
	puts(" (error ");
	printHex(inb(HD_ERR));
	puts("), controller 1: ");	
	printHex(inb(0x177));
	putch('\n');
	
	byte buf[512];
	for(i=0;i<512;++i) buf[i]=inb(HD_DATA);
	word *data = (word *)buf;
	if((data[0] & 0x8000)==1) puts("ATAPI device\n");
	else puts("ATA device\n");
	
	if(data[10] == 0) puts("No serial number\n");
	else {
		puts("Serial Number: ");
		for(i=10; i<20;++i) { printHex_w(data[i]); }
		putch('\n');
	}
	
	if(data[0x2E/2]==0) puts("No Firmware Revision\n");
	else {
		puts("Firmware Revision: ");
		for(i=0x2E/2; i<4+0x2E/2;++i) { printHex_w(data[i]); }
		putch('\n');
	}
	
	if(data[0x36/2]==0) puts("No Model number\n");
	else {
		puts("Model number: ");
		for(i=0x36/2; i<20+0x36/2;++i)  { printHex_w(data[i]); }
		putch('\n');
	}
	
	puts("Physical Cylinders : ");
	printInt(data[1]);
	puts(", Heads : ");
	printInt(data[3]);
	puts(", Sectors : ");
	printInt(data[6]);
	putch('\n');
	*/
	
	//------
	
	// -- END HARD DISK ACCESS TESTING ---

/*
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
*/
	
	/*
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
	*/
	
	// Test Division By 0
	// i = 10 / 0; putch(i);
	
	
	//
	settextcolor(0x0f, 0x00);
	puts("Starting Infinite Loop!\n");
	for (;;);
	return 0;
}

