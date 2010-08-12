#include <system.h>
#include <hd.h>
#include <multiboot.h>

// memcpy - copy n bytes from src to dest 
byte *memcpy(byte *dest, const byte *src, size_t count)
{
    const byte *sp = (const byte *)src;
    byte *dp = (byte *)dest;
    for(; count != 0; count--)
        *dp++ = *sp++;
    return dest;
}

// memset - set all bytes in a range of memory
byte *memset(byte *dest, byte val, size_t count)
{
    byte *temp = dest;
    for( ; count != 0; count--)
        *temp++ = val;
    return dest;
}

// memsetw - same as memset, but with word size chunks
uint16 *memsetw(uint16 *dest, uint16 val, size_t count)
{
    uint16 *temp = (uint16 *)dest;
    for( ; count != 0; count--)
        *temp++ = val;
    return dest;
}

// strlen - gets the length of a c-string
int strlen(const uint8 *str)
{
    size_t retval;
    for(retval = 0; *str != '\0'; str++)
        retval++;
    return retval;
}

// NASM assembly boot loader calls this method
int _main(multiboot_info_t* mbh, uint32 magic)
{
    //int z = 0;
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
    puts(" - http://www.osdev.org/\n");
    puts(" - http://www.osdever.net/bkerndev/index.php\n");
    puts("\n");
    settextcolor(0x03, 0x00);
    puts("This operating system is a test bed to \n");
    puts("experiment with writing an operating system kernel\n");
    puts("and possibly more\n");
    puts("\n");

    puts("Main Memory: ");
    printInt(mbh->mem_lower);
    puts("B (low), ");
    printInt(mbh->mem_upper);
    puts("B (high)\n");

//
//    puts("Start Ticks: "); printInt(timer_ticks()); putch('\n');
//    puts("Start Seconds: "); printInt(timer_seconds()); putch('\n');
//
//    for(i=0;i<10;i++) {
//    	//putch('.');
//    	delay_ms(1);
//    }
//
//    puts("End Ticks: "); printInt(timer_ticks()); putch('\n');
//    puts("End Seconds: "); printInt(timer_seconds()); putch('\n');


    getch();
    settextcolor(0x0f, 0x00);

    // -- BEG HARD DISK ACCESS TESTING ---

    ata_soft_reset();

    cli();
    // wait while not ready
    ata_wait_ready();
    outb(HD_DH, IDE0_BASE & 0xff);
    outb(HD_CMD, HD_CMD_IDENTIFY);
    ata_wait_drq();
    word ident_data[256];
    for(i=0;i<256; ++i) {
    	ident_data[i] = inw(HD_DATA);
    }
    sti();

    // 00 - Useful if not Hard Disk
    printHex_w(ident_data[0]);

    // 01,03,06 - CHS
    puts("\nC: ");printInt(ident_data[1]);
    puts(" H: ");printInt(ident_data[3]);
    puts(" S: ");printInt(ident_data[6]);
        
    uint32 bytes = chs2bytes(ident_data[1], ident_data[3], ident_data[6]);
    uint32 kilobytes = bytes/1024;
    uint32 megabytes = bytes/1048576;
    uint32 gigabytes = bytes/1073741824;
    
    puts("\nStorage Size is ");
    printInt(kilobytes); puts("KB, ");
    printInt(megabytes); puts("MB, ");
    printInt(gigabytes); puts("GB");

    // 10-19 - Serial Number
    puts("\nSerial: ");
    for(i=10;i<19;++i) {
    	putch((ident_data[i] >> 8) & 0xff);
    	putch(ident_data[i] & 0xff);
    }
    // 23-26 Firmware Revision
    puts("\nFirmware: ");
    for(i=23;i<26;++i) {
    	putch((ident_data[i] >> 8) & 0xff);
    	putch(ident_data[i] & 0xff);
    }
    // 27-46 - Model Name
    puts("\nModel: ");
    for(i=27;i<46;++i) {
    	putch((ident_data[i] >> 8) & 0xff);
    	putch(ident_data[i] & 0xff);
    }

    // 49 - (bit 9) LBA Supported
    if(ident_data[49] & 0x0100) puts("\nLBA Supported!");

    if(ident_data[59] & 0x0100) puts("\nMultiple sector setting is valid!");

    // 60/61 - taken as DWORD => total # LBA28 sectors (if > 0, supports LBA28)
    uint32 lba_capacity = (ident_data[61] << 16) + ident_data[60];
    uint32 lba_bytes = (lba_capacity/MEGA*SECTOR_BYTES);
    puts("\nLBA Capacity: ");printInt(lba_capacity);puts(" Sectors");
    puts("\nLBA Capacity: ");printInt(lba_bytes);puts("MB");



    if(ata_controller_present(0)){
		trace("\nController 0 EXISTS");
	} else {
		trace("\nController 0 NOT EXIST");
	}

    if(ata_controller_present(1)){
		trace(" Controller 1 EXISTS");
	} else {
		trace(" Controller 1 NOT EXIST");
	}

	//ata_soft_reset();

	if(ata_drive_present(0, 0)){
		trace("\nPri Drive 0 EXISTS");
	} else {
		trace("\nPri Drive 0 NOT EXIST");
	}

    if(ata_drive_present(0, 1)){
		trace(" Pri Drive 1 EXISTS");
	} else {
		trace(" Pri Drive 1 NOT EXIST");
	}

    if(ata_drive_present(1, 0)){
		trace("\nSec Drive 0 EXISTS");
	} else {
		trace("\nSec Drive 0 NOT EXIST");
	}

    if(ata_drive_present(1,1)){
		trace(" Sec Drive 1 EXISTS");
	} else {
		trace(" Sec Drive 1 NOT EXIST");
	}

	word data[512], data2[512];
    for(i=0;i<512;++i) data[i]=0x5A;

    ata_pio_write_w(0,1,1,1,data);
    putch('\n'); for(i=0;i<10;++i) printHex_w(data[i]);

    while((inb(HD_ST_ALT) & 0xc0) != 0x40);

    ata_pio_read_w(0,1,1,1,data2);
    putch('\n'); for(i=0;i<10;++i) printHex_w(data2[i]);

    getch();

    // -- END HARD DISK ACCESS TESTING ---

    /*
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
    		s[i*2] = i*2;
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

*/


    // Test Division By 0
    // need to hide the zero
    //z = 2-1; i = 10 / z; putch(i);

    //
    settextcolor(0x0f, 0x00);
    puts("\nStarting Infinite Loop!");
    for (;;)
        ;
    return 0;
}

