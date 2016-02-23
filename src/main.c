#include <system.h>
#include <hd.h>
#include <multiboot.h>

// memcpy - copy n bytes from src to dest
u8 *memcpy(u8* dest, const u8* src, u32 count)
{
    const u8* sp = (const u8*)src;
    u8* dp = (u8*)dest;
    for(; count != 0; count--)
        *dp++ = *sp++;
    return dest;
}

// memset - set all bytes in a range of memory
u8* memset(u8* dest, u8 val, u32 count)
{
    u8* temp = dest;
    while(count != 0) {
        *temp++ = val; --count;
    }
    return dest;
}

// memsetw - same as memset, but with word size chunks
u16* memsetw(u16* dest, u16 val, u32 count)
{
    u16* temp = (u16*)dest;
    while(count != 0) {
        *temp++ = val; --count;
    }
    return dest;
}

// strlen - gets the length of a c-string
u32 strlen(const u8* str)
{
    u32 retval = 0;
    while(*str != 0) {
        ++retval; ++str;
    }
    return retval;
}

#define internal static
internal unsigned long int next = 1;

// RAND_MAX assumed to be 32767
int rand(void) 
{
    next = next * 1103515245 + 12345;
    return (unsigned int)(next/65536) % 32768;
}

void srand(unsigned int seed)
{
    next = seed;
}

bool hasBit(u32 data, u32 bit) {
    return (data & (1 << bit)) != 0;
}

bool is_bit_set(unsigned value, unsigned bitindex)
{
    return (value & (1 << bitindex)) != 0;
}

// NASM assembly boot loader calls this method
u32 _main(multiboot_info_t* mbh, u32 magic)
{
    // //u32 z = 0;
    // u32 i = 0;
    // i++;
    // i--;

    gdt_install();
    idt_install();
    isrs_install();
    irq_install();

    init_serial();
    // TODO: serial_write[ln]
    write_serial("Hi QEMU!\r\n");

    init_mm();
    init_video();

    timer_install();
    keyboard_install();

    __asm__ __volatile__ ("sti");



//unsigned char real_time_array [128];



    puts("\n");
    settextcolor(COLOR_BLACK, COLOR_CYAN);
    puts("Tranby OS\n");
    settextcolor(COLOR_WHITE, COLOR_BLACK);
    puts("By Steve Tranby\n");
    puts("\n");
    settextcolor(COLOR_BLUE, COLOR_BLACK);
    puts("With Help from:");
    puts(" - http://www.osdev.org/\n");
    puts(" - http://www.osdever.net/bkerndev/index.php\n");
    puts("\n");
    settextcolor(COLOR_GREEN, COLOR_BLACK);
    puts("This operating system is a test bed to experiment with writing\n");
    puts("an operating system kernel and possibly more.\n");
    puts("\n");

    puts("[==Multiboot Info == ");
    printHex(magic);
    puts("]\n");
    puts("Mem: ");
    printInt(mbh->mem_lower);
    puts("-");
    printInt(mbh->mem_upper);
    puts("B");
    puts(", flags:");
    printHex(mbh->flags);
    puts(", ");

    for(s8 _bit=32; _bit>=0; --_bit) {
        putch(is_bit_set(mbh->flags, _bit) ? 'x' : '.');
        //putch(hasBit(mbh->flags, _bit) ? 'x' : '.');
        // if((mbh->flags >> _bit) & 0x0001) {
        //     putch('x');
        // } else {
        //     putch('.');
        // }
    }
    puts("\n");

    //if(hasBit(mbh->flags, 11)) 
    {
        puts("VBE Info: ");
        printHex(mbh->vbe_control_info);  puts(", ");
        printHex(mbh->vbe_mode_info);     puts(", ");
        printHex(mbh->vbe_mode);          puts(", ");
        printHex(mbh->vbe_interface_seg); puts(", ");
        printHex(mbh->vbe_interface_off); puts(", ");
        printHex(mbh->vbe_interface_len);
        puts("\n");


        // 
    }

    if(hasBit(mbh->flags, 9)) {
        puts("Bootloader Name: ");
        puts((s8*)mbh->boot_loader_name);
        puts("\n");
    }
    

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
    settextcolor(COLOR_WHITE, COLOR_BLACK);

    // -- BEG HARD DISK ACCESS TESTING ---

    ata_soft_reset();

    cli();
    // wait while not ready
    ata_wait_ready();
    outb(HD_DH, IDE0_BASE & 0xff);
    outb(HD_CMD, HD_CMD_IDENTIFY);
    ata_wait_drq();
    u16 ident_data[256];
    for(int i=0; i<256; ++i) {
    	ident_data[i] = inw(HD_DATA);
    }
    sti();

    // 00 - Useful if not Hard Disk
    printHex_w(ident_data[0]);

    // 01,03,06 - CHS
    puts("\nC: ");printInt(ident_data[1]);
    puts(" H: ");printInt(ident_data[3]);
    puts(" S: ");printInt(ident_data[6]);

    u32 bytes = chs2bytes(ident_data[1], ident_data[3], ident_data[6]);
    u32 kilobytes = bytes/1024;
    u32 megabytes = bytes/1048576;
    u32 gigabytes = bytes/1073741824;

    puts("\nStorage Size is ");
    printInt(kilobytes); puts("KB, ");
    printInt(megabytes); puts("MB, ");
    printInt(gigabytes); puts("GB");

    // 10-19 - Serial Number
    puts("\nSerial: ");
    for(int i=10; i<19; ++i) {
    	putch((ident_data[i] >> 8) & 0xff);
    	putch(ident_data[i] & 0xff);
    }
    // 23-26 Firmware Revision
    puts("\nFirmware: ");
    for(int i=23; i<26; ++i) {
    	putch((ident_data[i] >> 8) & 0xff);
    	putch(ident_data[i] & 0xff);
    }
    // 27-46 - Model Name
    puts("\nModel: ");
    for(int i=27; i<46; ++i) {
    	putch((ident_data[i] >> 8) & 0xff);
    	putch(ident_data[i] & 0xff);
    }

// TODO: ident_data should have a struct type instead
    // 49 - (bit 9) LBA Supported
    if(ident_data[49] & 0x0100) 
        puts("\nLBA Supported!");
    if(ident_data[59] & 0x0100) 
        puts("\nMultiple sector setting is valid!");

    // 60/61 - taken as DWORD => total # LBA28 sectors (if > 0, supports LBA28)
    u32 lba_capacity = (ident_data[61] << 16) + ident_data[60];
    u32 lba_bytes = (lba_capacity/MEGA*SECTOR_BYTES);
    
    // TODO: write("LBA Cap: %d Sectors, %d MB", lba_capacity, lba_bytes);
    puts("\nLBA Capacity: ");
    printInt(lba_capacity);
    puts(" Sectors");

    puts(", ");
    printInt(lba_bytes);
    puts(" MB");

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

	u16 data[512];    
    for(int i=0; i<512; ++i)
        data[i]=0x5A;

    ata_pio_write_w(0,1,1,1,data);
    putch('\n'); 
    for(int i=0; i<10; ++i)
        printHex_w(data[i]);

    // wait
    while((inb(HD_ST_ALT) & 0xc0) != 0x40)
        ;

    // NOTE: QEMU protects block 0 from being written to if img format is unknown or raw
    u16 data2[512];
    ata_pio_read_w(0,1,1,1,data2);
    putch('\n'); 
    for(int i=0; i<10; ++i) 
        printHex_w(data2[i]);

    getch();

    // -- END HARD DISK ACCESS TESTING ---

    /*
    	int * p = 0;
    	puts("\nThe address of p is: ");
    	printInt((int)&p);
    	// DEBUG::Testing heap code
    	puts("\nHeap Magic: ");
    	print_heap_magic();

    	u8 *t = kmalloc(4);
    	u8 *s = kmalloc(8);
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
    

    delay_s(2);

    // TODO: k_println("{} and {}", a, b);

    putch('\n');

    // TEST Setup VGA Graphics mode
    //set_video_mode(video_mode_13h);
    //set_video_mode(video_mode_13h)
    int success = init_graph_vga(320,200,1);
    if(success) 
    {
        write_serial("Video Mode Success!\r\n");
        puts("Video Mode Success!\n");
        vga_tests();
    }


    // test modeX (non-chain4 allows 400x600 possible)
    success = init_graph_vga(320,240,0);
    if(success) 
    {
        write_serial("Video Mode Success!\r\n");
        puts("Video Mode Success!\n");
        vga_tests();
    }

// TODO: allow switching "stdout" from direct to serial or both
    rtc_time time = read_rtc();
    puts("datetime = "); 
    printInt(time.year);putch('-');
    printInt(time.month);putch('-');
    printInt(time.day);putch(' ');
    printInt(time.hour);putch(':');
    printInt(time.minute);putch(':');
    printInt(time.second);
    putch('\n');

    // TODO: fork off shell process
    write_serial("Starting Infinite Loop!\r\n");
    puts("Starting Infinite Loop!\n");

    for (;;)
        ;

    return 0;
}

