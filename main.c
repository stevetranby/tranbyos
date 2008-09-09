#include <system.h>
#include <hd.h>
#include <multiboot.h>

/* You will need to code these up yourself!  */
byte *memcpy(byte *dest, const byte *src, size_t count)
{
    const byte *sp = (const byte *)src;
    byte *dp = (byte *)dest;
    for(; count != 0; count--)
        *dp++ = *sp++;
    return dest;
}

byte *memset(byte *dest, byte val, size_t count)
{
    /* Add code here to set 'count' bytes in 'dest' to 'val'.
    *  Again, return 'dest' */

    byte *temp = dest;
    for( ; count != 0; count--)
        *temp++ = val;
    return dest;
}

uint16 *memsetw(uint16 *dest, uint16 val, size_t count)
{
    /* Same as above, but this time, we're working with a 16-bit
    *  'val' and dest pointer. Your code can be an exact copy of
    *  the above, provided that your local variables if any, are
    *  uint16 */
    uint16 *temp = (uint16 *)dest;
    for( ; count != 0; count--)
        *temp++ = val;
    return dest;
}

int strlen(const uint8 *str)
{
    /* This loops through character array 'str', returning how
    *  many characters it needs to check before it finds a 0.
    *  In simple words, it returns the length in bytes of a string */
    size_t retval;
    for(retval = 0; *str != '\0'; str++)
        retval++;
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

word inw(word _port)
{
    word rv;
__asm__ volatile ("inw %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

dword ind(word _port)
{
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
    i++;
    i--;

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

	// setup data
    word data[512];
    data[0] = 0x50;
    data[1] = 0x51;
    data[2] = 0x52;
    data[3] = 0x51;
    data[4] = 0x50;

//	// prep the controller
//    outb(HD_ST,0x0);
//    soft_reset();       
//    hd_print_status();
//    putch('\n');
//     
//    // get device type
//    outb(HD_DH, 0xA0);
//    hd_delay400ns();    
//    byte cl = inb(HD_CL);
//    byte ch = inb(HD_CH);    
//    puts("Device Type: ");
//    if(cl==0x14 && ch==0xEB) puts("PATAPI");
//    if(cl==0x69 && ch==0x96) puts("SATAPI");
//    if(cl==0x00 && ch==0x00) puts("ATA");
//    if(cl==0x3c && ch==0xc3) puts("SATA");
//    putch('\n');

    // setup command block registers
    outb(0x1F6, 0xA0); // primary master, head 0
    outb(0x1F2, 0x01);
    outb(0x1F3, 0x01);
    outb(0x1F4, 0x00);
    outb(0x1F5, 0x00);
    outb(0x1F7, 0x30);
    
    while( !(inb(0x1F7) & 0x08) ) hd_print_status();
    
    puts("Writing data: ");
    for(i=0; i<256; ++i) {
        outw(HD_DATA, data[i]);
        hd_print_status();
    }
    putch('\n');
    puts("Finished Writing");
    for(i=0; i<5; ++i)
    {
        printHex(data[i]);
        putch(' ');
    }
    putch('\n');
    
    outb(0x1F6, 0xA0); // primary master, head 0
    outb(0x1F2, 0x01);
    outb(0x1F3, 0x01);
    outb(0x1F4, 0x00);
    outb(0x1F5, 0x00);
    outb(0x1F7, 0x20);
    
    while( !(inb(0x1F7) & 0x08) ) hd_print_status();
    
    puts("Reading data: ");
    for(i=0; i<256; ++i) {
       	data[i] = inw(HD_DATA);
        hd_print_status();
    }
    putch('\n');
    puts("Finished Reading");
    for(i=0; i<5; ++i)
    {
        printHex(data[i]);
        putch(' ');
    }
    putch('\n');

//    hd_delay400ns();
//
//    if(inb(HD_ST) & HD_ST_ERR)
//    {
//        puts("Error: ");
//        hd_print_error();
//        putch('\n');
//    }
//     wait until DRQ is set
//    puts("Wait not busy: ");
//    while((inb(HD_ST) & (HD_ST_BSY))==HD_ST_BSY)
//        hd_print_status();
//    hd_print_status();
//    putch('\n');
//    puts("Wait until DRQ: ");
//    while((inb(HD_ST) & (HD_ST_DRQ))!=HD_ST_DRQ)
//        hd_print_status();
//    hd_print_status();
//    putch('\n');
//    puts("Wait until RDY: ");
//    while((inb(HD_ST) & (HD_ST_RDY))!=HD_ST_RDY)
//        hd_print_status();
//    hd_print_status();
//    putch('\n');
//
//    if(inb(HD_ST) & HD_ST_ERR)
//    {
//        puts("Error: ");
//        hd_print_error();
//        putch('\n');
//    }
//
//    puts("Writing data: ");
//    for(i=0; i<512; ++i) {
//        outb(HD_DATA, 0x50);
//        hd_print_status();
//    }
//    
//    puts("Wait not busy: ");
//    while((inb(HD_ST) & (HD_ST_BSY))==HD_ST_BSY)
//    {
//        hd_print_status();
//        if(inb(HD_ST) & HD_ST_ERR)
//        {
//            puts("Error: ");
//            hd_print_error();
//            putch('\n');
//        }
//    }
//
//    hd_delay400ns();
//    hd_print_status();
//    putch('\n');
//
//    for(i=0; i<5; ++i)
//    {
//        printHex(data[i]);
//        putch(' ');
//    }
//	putch('\n');
//	
//   	outb(0x1f7, 0x00);
//    hd_delay400ns();
//
//     setup command block registers
//    outb(0x1F2, 0x01);
//    outb(0x1F3, 0x01);
//    outb(0x1F4, 0x00);
//    outb(0x1F5, 0x00);
//    outb(0x1F6, 0xA0); // primary master, head 0
//    outb(0x1F7, 0x20);
//
//    hd_delay400ns();
//
//     wait until DRQ is set
//    puts("Wait not busy: ");
//    while((inb(HD_ST) & (HD_ST_BSY))==HD_ST_BSY)
//    {
//        hd_print_status();
//        if(inb(HD_ST) & HD_ST_ERR)
//        {
//            puts("Error: ");
//            hd_print_error();
//            putch('\n');
//        }
//    }
//    hd_print_status();
//    putch('\n');
//    puts("Wait until DRQ: ");
//    while((inb(HD_ST) & (HD_ST_DRQ))!=HD_ST_DRQ)
//    {
//        hd_print_status();
//        if(inb(HD_ST) & HD_ST_ERR)
//        {
//            puts("Error: ");
//            hd_print_error();
//            putch('\n');
//        }
//    }
//
//    hd_print_status();
//    putch('\n');
//    puts("Wait until RDY: ");
//    while((inb(HD_ST) & (HD_ST_RDY))!=HD_ST_RDY)
//    {
//        hd_print_status();
//        if(inb(HD_ST) & HD_ST_ERR)
//        {
//            puts("Error: ");
//            hd_print_error();
//            putch('\n');
//        }
//    }
//    hd_print_status();
//    putch('\n');
//
//    puts("Reading data: ");
//    for(i=0; i<512; ++i)
//        data[i] = inb(HD_DATA);
//
//    hd_delay400ns();
//    hd_print_status();
//    putch('\n');
//
//    for(i=0; i<5; ++i)
//    {
//        printHex(data[i]);
//        putch(' ');
//    }

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
    puts("\nStarting Infinite Loop!\n");
    for (;;)
        ;
    return 0;
}

