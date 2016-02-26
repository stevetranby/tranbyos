// /**/
//
// Stream Setup and Prep:
// - Practice Streaming Something Simple
// - Decide on one or two major features "unique" to the OS
// - Create a list of topics for the first 10 streams
// - Decide on rough time limit, stick to it as close as possible
// - Setup Patreon
//
// Stream Schedule (Actual Topics):
// - ?? follow potential list when don't have schedule set ??
//
// Stream Topics (Potential Topics):
// - OSX Developer Environment
// - Linux Developer Environment (Debian apt-get, Arch pacman)
// - Win10 Developer Environment
// - Virtual Machine, QEMU, Bochs, VirtualBox (VMWare,Parallels,Docker,Vagrant)
// - Bootloader v1
//     - real mode
//     - print hello world
//     - simple 'shell' with couple commands
// - NASM assmebler (Intel Syntax)
// - Inline Assembly (AT&T Syntax)
// - Bootloaders v2 (GRUB, MultiBoot)
//     - Show how GRUB2 bootloader works (TranbyOS 0.1)
//     - Setting up GRUB-based kernel
//     - Set up GRUB Long Mode Kernel
// - Interrupts (IRQs, ISRs, GDT, IDT, etc)
// - Processor Modes (Real, Protected, Long)
// - Bootloader v3 (Protected Mode)
//     - Show how most of kernel from v2 is the same
//      - no more segments, flat addressing
// - Bootloader v4 (Long Mode)
//      - differences with 64-bit mode?
//      - show going back and forth from Real<-->Protected<-->Long
// - PIO ATA Hard Disk Read/Write
// - DMA ATA Hard Disk Read/Write
//
// - Process Structure
// - Memory Manager
// - Memory Paging and Page Table
// - Caching and TLB
// - CDROM Read/Write??
// - x86, x86_64, Arm64
//
//
// Ideas:
// - allow building and run on
//
//
// References:
//
// - Cryptographically Secure Pseudo-RNG (CSPNG):
//      https://en.wikipedia.org/wiki/Cryptographically_secure_pseudorandom_number_generator
// - arc4random uniform distribution:
//      http://www.openbsd.org/cgi-bin/man.cgi/OpenBSD-current/man3/arc4random.3?query=arc4random&sec=3
//
//
// TODO(steve):
// - Decide on Kernel Name, OS Name, codenames?
//      ASCII ART:
//          http://chris.com/ascii/index.php
//          http://www.patorjk.com/software/taag/#p=testall&f=Lil%20Devil&t=TranbyOS
// - Linux Kernel v0.1:
//      https://www.kernel.org/pub/linux/kernel/Historic/
// - Switch Processing Modes: Real <--> Protected <--> Long
//      http://www.codeproject.com/Articles/45788/The-Real-Protected-Long-mode-assembly-tutorial-for
// - CPUID:
//      https://github.com/rythie/CPUID-in-Assembly
// - Processor TLB Structure
// - Context Switch Capability
// - Fork Capability
// - Memory Allocator (malloc-like, free store, index forward and circle around, over time bring all blocks to the latest HEAD)
//   - see the video somewhere on SSD's not like erase, but write okay
// - Simple Shell (trash - TRAnby SHell)
//
//
// TODO(Future):
//
// - Build on Win10 Support
// -

// Regular Experession
// find: ^/////+///$
// repl: //////////////////////////////////////////////////////////////////
//
//
//

// # Completed

#include "include/system.h"

// Grub2
#include "include/multiboot.h"

// memcpy - copy n bytes from src to dest
u8* memcpy(u8* dest, const u8* src, u32 count)
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
u32 strlen(c_str str)
{
    u32 retval = 0;
    while(*str != 0) {
        ++retval; ++str;
    }
    return retval;
}

// RAND_MAX assumed to be 32767
static u32 _next_rand = 1;
u32 rand(void)
{
    _next_rand = _next_rand * 1103515245 + 12345;
    return (_next_rand/65536) % 32768;
}

void srand(u32 seed)
{
    _next_rand = seed;
}

b32 hasBit(u32 data, u32 bit) {
    return (data & (1 << bit)) != 0;
}

b32 is_bit_set(u32 value, u32 bitindex)
{
    return (value & (1 << bitindex)) != 0;
}

void wait_any_key() {
    set_text_color(COLOR_LIGHT_MAGENTA, COLOR_BLACK);

    // TODO: clear_key_buffer();
    serial_write("Clearing Key Buffer\n");
    while(keyboard_read_next())
        putch('.');

    serial_write("Waiting for Key\n");
    puts("Press Any Key (muahahaha!!!!)");
    getch();

    set_text_color(COLOR_WHITE, COLOR_BLACK);

    puts("\n");
}

void display_banner()
{
    set_text_color(COLOR_GREEN, COLOR_BLACK);

    /// http://www.patorjk.com/software/taag/#p=testall&f=Lil%20Devil&t=TranbyOS
    ///
    puts("                                                                            \n");
    puts(" _/_/_/_/_/                            _/                   _/_/     _/_/_/ \n");
    puts("    _/     _/  _/_/   _/_/_/ _/_/_/   _/_/_/    _/    _/ _/    _/ _/        \n");
    puts("   _/     _/_/     _/    _/ _/    _/ _/    _/  _/    _/ _/    _/   _/_/     \n");
    puts("  _/     _/       _/    _/ _/    _/ _/    _/  _/    _/ _/    _/       _/    \n");
    puts(" _/     _/         _/_/_/ _/    _/ _/_/_/      _/_/_/   _/_/   _/_/_/       \n");
    puts("                                                 _/                         \n");
    puts("                                            _/_/                            \n");
    puts("                                                                            \n");

    //puts("\n-= Tranby OS =-\n");

    //set_text_color(COLOR_WHITE, COLOR_BLACK);

    puts("Steve Tranby (stevetranby@gmail.com)\n");
    puts("http://stevetranby.com/\n");
    puts("http://github.com/stevetranby/tranbyos\n");
    puts("http://osdev.org/\n");
    puts("\n");
    puts("This operating system is a test bed for experimenting and learning how to\n");
    puts("write an operating system kernel, drivers, and possibly more.\n");
    puts("\n");
}

// NASM assembly boot loader calls this method
u32 _main(multiboot_info_t* mbh, u32 magic)
{
    gdt_install();
    idt_install();
    isrs_install();

    init_serial();
    serial_write("Hi QEMU!\r\n");

    init_mm();
    init_video();

    timer_install();
    keyboard_install();
    mouse_install();

    sti();

// TODO
//    u32 timestamp = rsptd();
//    srand();

    display_banner();


    set_text_color(COLOR_LIGHT_RED, COLOR_BLACK);

    // Multiboot Info
    puts("MBInfo: ");
    printHex(magic);
    puts(", Mem: ");
    printInt(mbh->mem_lower);
    puts("-");
    printInt(mbh->mem_upper);
    puts("B");
    puts(", Flags:");
    printHex(mbh->flags);
    puts(", ");
    puts("\n");

    // TODO: do VBE in real mode
    if(hasBit(mbh->flags, 11))
    {
        puts("VBE Info: ");
        printHex(mbh->vbe_control_info);  puts(", ");
        printHex(mbh->vbe_mode_info);     puts(", ");
        printHex(mbh->vbe_mode);          puts(", ");
        printHex(mbh->vbe_interface_seg); puts(", ");
        printHex(mbh->vbe_interface_off); puts(", ");
        printHex(mbh->vbe_interface_len);
        puts("\n");
    }

    if(hasBit(mbh->flags, 9)) {
        puts("Bootloader Name: ");
        puts((i8*)mbh->boot_loader_name);
        puts("\n");
    }


    u32 ticks = timer_ticks();
    puts("Start Timer");
    for(int i=0;i<10;i++) {
        putch('.');
        delay_ms(100);
    }
    puts("Done. ");
    printInt(timer_ticks() - ticks);
    puts("Ticks\n");

    wait_any_key();


    set_text_color(COLOR_MAGENTA, COLOR_BLACK);



    // -- BEG HARD DISK ACCESS TESTING ---

    ata_soft_reset();

    // wait while not ready
    cli();
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
    puts("Disk: ");
    printHex_w(ident_data[0]);

    // 01,03,06 - CHS
    puts(", Cyl:"); printInt(ident_data[1]);
    puts(", Heads:");  printInt(ident_data[3]);
    puts(", Sectors:");  printInt(ident_data[6]);

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
    putch('\n');

    wait_any_key();

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
    putch('\n');

    // -- END HARD DISK ACCESS TESTING ---


    wait_any_key();


    set_text_color(COLOR_YELLOW, COLOR_BLACK);


    int * p = 0;
    puts("\nThe address of p is: ");
    printAddr(&p);
    // DEBUG::Testing heap code
    puts("\nHeap Magic: ");
    print_heap_magic();

    u8 *t = kmalloc(4);
    u8 *s = kmalloc(8);
    for(int i=0; i<4; ++i) {
        t[i] = i;
        s[i*2] = i*2;
        s[i*2+1] = i*2+1;
    }
    for(int i=0; i<4; ++i) {
        printAddr(t);
        putch(':');
        printInt(t[i]);
        puts(", ");
        printAddr(s);
        putch(':');
        printInt(s[i*2]);
        putch(':');
        printInt(s[i*2+1]);
        putch('\n');
    }

    // print out some memory
    putch('\n');
    int array[10] = { 1,1,2,3,5,8,13,21,34 };
    p = array;
    for(int i=0; i<10; ++i, ++p) {
        printInt((int)&p);
        putch(':');
        printInt((int)*p);
        puts("  ");
        if(i % 7 == 6) puts("\n");
    }

    // Test Division By 0
    // need to hide the zero
    //z = 2-1; i = 10 / z; putch(i);

    //
    putch('\n');

    // TODO: k_println("{} and {}", a, b);


    wait_any_key();




    // TODO: allow switching "stdout" from direct to serial or both
    rtc_time time = read_rtc();
    puts("datetime = ");
    //    if(time.century > 0) {
    //        printInt(time.century);
    if(time.year > 100) {
        printInt(time.year);
    } else {
        printInt(time.year + 1970);
        puts("[1970]");
    }
    putch('-');
    if(time.month < 10)
        putch('0');
    printInt(time.month);
    putch('-');
    if(time.day < 10)
        putch('0');
    printInt(time.day);
    putch(' ');
    if(time.hour < 10)
        putch('0');
    printInt(time.hour);
    putch(':');
    if(time.minute < 10)
        putch('0');
    printInt(time.minute);
    putch(':');
    if(time.second < 10)
        putch('0');
    printInt(time.second);
    putch('\n');

    // TODO: fork off shell process
    serial_write("Starting Infinite Loop!\r\n");
    puts("Starting Infinite Loop!\n");



    puts("Press SPACE BAR:");
    u8 SCAN_SPACE = 0x39;
    while(SCAN_SPACE != keyboard_read_next())
        ;

    // TEST Setup VGA Graphics mode
    //set_video_mode(video_mode_13h);
    //set_video_mode(video_mode_13h)
    u32 success = init_graph_vga(320,200,1);
    if(success)
    {
        serial_write("["__DATE__" "__TIME__"] Video Mode Success!\r\n");
        vga_tests();
    }
    
    
    // test modeX (non-chain4 allows 400x600 possible)
    success = init_graph_vga(320,240,0);
    if(success) 
    {
        serial_write("["__DATE__" "__TIME__"] Video Mode Success!\r\n");
        vga_tests();
    }
    
    
    for (;;)
        ;
    
    return 0;
}

