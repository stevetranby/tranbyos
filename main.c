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
    
    getch();
	settextcolor(0x0f, 0x00);
	
    // -- BEG HARD DISK ACCESS TESTING ---
    
    ata_soft_reset();
    
    if(ata_controller_present(0)){
		puts("\nController 0 EXISTS");
	} else {
		puts("\nController 0 NOT EXIST");
	}	
	
    if(ata_controller_present(1)){
		puts(" Controller 1 EXISTS");
	} else {
		puts(" Controller 1 NOT EXIST");
	}	
	
	if(ata_drive_present(0, 0)){
		puts("\nController 0 Drive 0 EXISTS");
	} else {
		puts("\nController 0 Drive 0 NOT EXIST");
	}	
	
    if(ata_drive_present(0, 1)){
		puts(" Controller 0 Drive 1 EXISTS");
	} else {
		puts(" Controller 0 Drive 1 NOT EXIST");
	}	
	
    if(ata_drive_present(1, 0)){
		puts("\nController 1 Drive 0 EXISTS");
	} else {
		puts("\nController 1 Drive 0 NOT EXIST");
	}	
	
    if(ata_drive_present(1,1)){
		puts(" Controller 1 Drive 1 EXISTS");
	} else {
		puts(" Controller 1 Drive 1 NOT EXIST");
	}	
	
	word data[512], data2[512];
    for(i=0;i<512;++i) data[i]=0x5A;
    ata_pio_write_w(0,0,1,data,1);
    ata_pio_read_w(0,0,1,data2,1);
	
    getch();
    // -- END HARD DISK ACCESS TESTING ---

    
    	int * p = 0;
    	puts("\nThe address of p is: ");
    	printInt((int)&p);    	
    	// DEBUG::Testing heap code
    	puts("\nHeap Magic: ");
    	print_heap_magic();    	
    	
    	byte *t = kmalloc(4);
    	byte *s = kmalloc(8);
    	for(i=0; i<4; ++i) {
    		t[i] = i;
    		s[i*2] = i*2;
    		s[i*2+1] = i*2+1;
    	}
    	puts(" -- ");
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
        
    puts("\nSize of RAM Lower=");
    printInt((int)mbd->mem_lower * 1024);
    puts(" Upper=");
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
    
    //
    settextcolor(0x0f, 0x00);
    puts("\nStarting Infinite Loop!");
    for (;;)
        ;
    return 0;
}

