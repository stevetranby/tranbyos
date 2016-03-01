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
// swap params (simple)
// find: \((\w+\s+\w+), (\w+\s+\w+)\)
// repl: \($2, $1\)
//
// # Completed

// TODO:
// - https://gcc.gnu.org/onlinedocs/libstdc++/manual/using_headers.html

// Require libc
// #include <stdint.h>

/* Check if the compiler thinks we are targeting the wrong operating system. */
#if defined(__linux__)
    #error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif
 
/* This tutorial will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
    #error "This tutorial needs to be compiled with a i386-elf compiler"
#endif

#include <system.h>

// Grub2
#include "include/multiboot.h"


/////////////////////////////

#define ALIGN (sizeof(size_t))
#define ONES ((size_t)-1/UCHAR_MAX)
#define HIGHS (ONES * (UCHAR_MAX/2+1))
#define HASZERO(X) (((X)-ONES) & ~(X) & HIGHS)

#define BITOP(A, B, OP) \
((A)[(size_t)(B)/(8*sizeof *(A))] OP (size_t)1<<((size_t)(B)%(8*sizeof *(A))))

void* kmemcpy(void* restrict dest, const void* restrict src, size_t n)
{
    asm volatile("rep movsb"
                 : "=c"((i32){0})
                 : "D"(dest), "S"(src), "c"(n)
                 : "flags", "memory");
    return dest;
}

void* kmemset(void* dest, int c, size_t n)
{
    asm volatile("rep stosb"
                 : "=c"((int){0})
                 : "D"(dest), "a"(c), "c"(n)
                 : "flags", "memory");
    return dest;
}

size_t kmemcmp(const void* vl, const void* vr, size_t n)
{
    const u8* l = vl;
    const u8* r = vr;
    for (; n && *l == *r; n--, l++, r++);
    return n ? *l-*r : 0;
}

void* kmemchr(const void * src, int c, size_t n) {
    const unsigned char * s = src;
    c = (unsigned char)c;
    for (; ((uintptr_t)s & (ALIGN - 1)) && n && *s != c; s++, n--);
    if (n && *s != c) {
        const size_t * w;
        size_t k = ONES * c;
        for (w = (const void *)s; n >= sizeof(size_t) && !HASZERO(*w^k); w++, n -= sizeof(size_t));
        for (s = (const void *)w; n && *s != c; s++, n--);
    }
    return n ? (void *)s : 0;
}

void* kmemrchr(const void * m, int c, size_t n)
{
    const unsigned char * s = m;
    c = (unsigned char)c;
    while (n--) {
        if (s[n] == c) {
            return (void*)(s+n);
        }
    }
    return 0;
}

void* kmemmove(void * dest, const void * src, size_t n)
{
    char * d = dest;
    const char * s = src;

    if (d==s) {
        return d;
    }

    if (s+n <= d || d+n <= s) {
        return kmemcpy(d, s, n);
    }

    if (d<s) {
        if ((uintptr_t)s % sizeof(size_t) == (uintptr_t)d % sizeof(size_t)) {
            while ((uintptr_t)d % sizeof(size_t)) {
                if (!n--) {
                    return dest;
                }
                *d++ = *s++;
            }
            for (; n >= sizeof(size_t); n -= sizeof(size_t), d += sizeof(size_t), s += sizeof(size_t)) {
                *(size_t *)d = *(size_t *)s;
            }
        }
        for (; n; n--) {
            *d++ = *s++;
        }
    } else {
        if ((uintptr_t)s % sizeof(size_t) == (uintptr_t)d % sizeof(size_t)) {
            while ((uintptr_t)(d+n) % sizeof(size_t)) {
                if (!n--) {
                    return dest;
                }
                d[n] = s[n];
            }
            while (n >= sizeof(size_t)) {
                n -= sizeof(size_t);
                *(size_t *)(d+n) = *(size_t *)(s+n);
            }
        }
        while (n) {
            n--;
            d[n] = s[n];
        }
    }
    
    return dest;
}

////////////////////////////////

// memcpy - copy n bytes from src to dest
u8* kmemcpyb(u8* dest, const u8* src, u32 count)
{
    const u8* sp = (const u8*)src;
    u8* dp = (u8*)dest;
    for(; count != 0; count--)
        *dp++ = *sp++;
    return dest;
}

u8* kmemsetb(u8* dest, u8 val, u32 count)
{
    u8* temp = dest;
    while(count != 0) {
        *temp++ = val; --count;
    }
    return dest;
}

u16* kmemsetw(u16* dest, u16 val, u32 count)
{
    u16* temp = (u16*)dest;
    while(count != 0) {
        *temp++ = val; --count;
    }
    return dest;
}

// strlen - gets the length of a c-string
u32 kstrlen(c_str str)
{
    u32 retval = 0;
    while(*str != 0) {
        ++retval; ++str;
    }
    return retval;
}

// RAND_MAX assumed to be 32767
static u32 _next_rand = 1;
u32 krand(void)
{
    _next_rand = _next_rand * 1103515245 + 12345;
    return (_next_rand/65536) % 32768;
}

void srand(u32 seed)
{
    _next_rand = seed;
}

#define BIT(data,bit) (data & (1 << bit))

void wait_any_key() {
    set_text_color(COLOR_LIGHT_MAGENTA, COLOR_BLACK);

    // TODO: clear_key_buffer();
    serial_write("Clearing Key Buffer\n");
    while(keyboard_read_next()) {
        kputch('.');
        serial_write_b('.');
    }

    serial_write("Waiting for Key\n");
    kputs("Press Any Key (muahahaha!!!!)");
    kgetch();

    set_text_color(COLOR_WHITE, COLOR_BLACK);

    kputs("\n");
}

void display_banner()
{
    set_text_color(COLOR_GREEN, COLOR_BLACK);

    /// http://www.patorjk.com/software/taag/#p=testall&f=Lil%20Devil&t=TranbyOS
    ///
    kputs("                                                                            \n");
    kputs(" _/_/_/_/_/                            _/                   _/_/     _/_/_/ \n");
    kputs("    _/     _/  _/_/   _/_/_/ _/_/_/   _/_/_/    _/    _/ _/    _/ _/        \n");
    kputs("   _/     _/_/     _/    _/ _/    _/ _/    _/  _/    _/ _/    _/   _/_/     \n");
    kputs("  _/     _/       _/    _/ _/    _/ _/    _/  _/    _/ _/    _/       _/    \n");
    kputs(" _/     _/         _/_/_/ _/    _/ _/_/_/      _/_/_/   _/_/   _/_/_/       \n");
    kputs("                                                 _/                         \n");
    kputs("                                            _/_/                            \n");
    kputs("                                                                            \n");

    //kputs("\n-= Tranby OS =-\n");

    //set_text_color(COLOR_WHITE, COLOR_BLACK);

    kputs("Steve Tranby (stevetranby@gmail.com)\n");
    kputs("http://stevetranby.com/\n");
    kputs("http://github.com/stevetranby/tranbyos\n");
    kputs("http://osdev.org/\n");
    kputs("\n");
    kputs("This operating system is a test bed for experimenting and learning how to\n");
    kputs("write an operating system kernel, drivers, and possibly more.\n");
    kputs("\n");
}

#define my_test(x) _Generic((x), long double: my_testl, \
                                 default: my_testi, \
                                 float: my_testf)(x)

void my_testi(int i) { UNUSED_PARAM(i); kputs("int\n"); }
void my_testf(float f) { UNUSED_PARAM(f); kputs("float\n"); }
void my_testl(long double l) { UNUSED_PARAM(l); kputs("long double\n"); }

// NASM assembly boot loader calls this method
u32 _kmain(multiboot_info* mbh, u32 magic)
{
    gdt_install();
    idt_install();
    isrs_install();

    init_serial();
    serial_write("Hi QEMU!\r\n");

    // kernel subsystems
    init_mm();
    //TODO: init_tasking();
    //TODO: init_paging();
    //TODO:

    init_video();

    timer_install();
    keyboard_install();
    mouse_install();

    sti();





    if(BIT(mbh->flags, 11))
    {
        // https://codereview.stackexchange.com/questions/108168/vbe-bdf-font-rendering
        vbe_mode_info* vbe = (vbe_mode_info*)mbh->vbe_mode_info;
        kwritef(serial_write_b, "attributes: %u\nwinA: %u\nwinB: %u\ngranularity: %u\nwinsize: %u\nsegmentA: %x\nsegmentB: %x\nwinFuncPtr: %x\npitch: %u\nXres: %u\nYres: %u\nWchar: %u\nYchar: %u\nplanes: %u\nbpp: %u\nbanks: %u\nmemory_model: %u\nbank_size: %u\nimage_pages: %u\nreserved0: %u\nred_mask_size: %u\nred_position: %u\ngreen_mask_size: %u\ngreen_position: %u\nblue_mask_size: %u\nblue_position: %u\nrsv_mask: %u\nrsv_position: %u\ndirectcolor_attributes: %u\nphysbase: %x\nreserved1: %u\nreserved2: %u\nLinBytesPerScanLine: %u\nBnkNumberofImagePages: %u\nLinNumberofImagePages: %u\nLinRedMaskSize: %u\nLinRedFieldPosition: %u\nLinGreenMaskSize: %u\nLinGreenFieldPosition: %u\nLinBlueMaskSize: %u\nLinBlueMaskPosition: %u\nLinRsvdMaskSize: %u\nLinRsvdFieldPosition: %u\nMaxPixelClock: %u\nReserved: %u",
                vbe->attributes,
                vbe->winA,
                vbe->winB,
                vbe->granularity,
                vbe->winsize,
                vbe->segmentA,
                vbe->segmentB,
                vbe->winFuncPtr,
                vbe->pitch,
                vbe->Xres,
                vbe->Yres,
                vbe->Wchar,
                vbe->Ychar,
                vbe->planes,
                vbe->bpp,
                vbe->banks,
                vbe->memory_model,
                vbe->bank_size,
                vbe->image_pages,
                vbe->reserved0,
                vbe->red_mask_size,
                vbe->red_position,
                vbe->green_mask_size,
                vbe->green_position,
                vbe->blue_mask_size,
                vbe->blue_position,
                vbe->rsv_mask,
                vbe->rsv_position,
                vbe->directcolor_attributes,
                vbe->physbase,
                vbe->reserved1,
                vbe->reserved2,
                vbe->LinBytesPerScanLine,
                vbe->BnkNumberofImagePages,
                vbe->LinNumberofImagePages,
                vbe->LinRedMaskSize,
                vbe->LinRedFieldPosition,
                vbe->LinGreenMaskSize,
                vbe->LinGreenFieldPosition,
                vbe->LinBlueMaskSize,
                vbe->LinBlueMaskPosition,
                vbe->LinRsvdMaskSize,
                vbe->LinRsvdFieldPosition,
                vbe->MaxPixelClock,
                vbe->Reserved);


        //kmemset((u32*)vbe->physbase, 0xff2222, 2^16);

        uint16_t uses_lfb = BIT(mbh->vbe_mode, 14);
        if (uses_lfb)
            serial_write("uses LFB");
        int32_t colors[] = {
            0xFFFFFF, 0xC0C0C0, 0x808080,
            0x000000, 0xFF0000, 0x800000,
            0xFFFF00, 0x808000, 0x00FF00,
            0x008000, 0x00FFFF, 0x008080,
            0x0000FF, 0x000080, 0xFF00FF,
            0x800080
        };


        //#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))
        //vbe_mode_info_t *vbe;
        uint8_t *mem = (uint8_t *)vbe->physbase;

        // 24bpp
        void draw_pixel(int x, int y, uint32_t color)
        {
            u16 pos = y * vbe->LinBytesPerScanLine + x * (vbe->bpp / 8);
            mem[pos] = color & 0xFF;
            mem[pos + 1] = (color >> 8) & 0xFF;
            mem[pos + 2] = (color >> 16) & 0xFF;
        }

        void draw_rectangle(int x, int y, int width, int height, uint32_t color)
        {
            for (int i = x; i < x + width; ++i)
            {
                for (int j = y; j < y + height; ++j)
                {
                    draw_pixel(i, j, color);
                }
            }
        }
        
        int idx = 0;
        int chunk_size = 8;
        int xchunk = (vbe->Xres / chunk_size);
        int ychunk = (vbe->Yres / chunk_size);
        for (int i = 0; i < chunk_size; ++i)
        {
            for (int j = 0; j < chunk_size; ++j)
            {
                draw_rectangle(j * xchunk, i * ychunk, xchunk, ychunk,
                               colors[idx]);
                if (++idx == 16)
                    idx = 0;
            }
        }
    }
    
    // TODO: do VBE in real mode
    {
        // Multiboot Info
        kwritef(serial_write_b, "MBInfo: %x, Mem: %d-%dB, Flags: %x\n",
                magic, mbh->mem_lower, mbh->mem_upper, mbh->flags);
        kwritef(serial_write_b, "VBE: %x,%x,%x,%x,%x\n",
                mbh->vbe_control_info,
                mbh->vbe_mode_info,
                mbh->vbe_mode,
                mbh->vbe_interface_seg,
                mbh->vbe_interface_off,
                mbh->vbe_interface_len);
        kwritef(serial_write_b,"MMap:\t%x\nAddr:\t%x\nDrives:\t%xAddr:\t%x\nConfig:\t%x\n",
                mbh->mmap_length,
                mbh->mmap_addr,
                mbh->drives_length,
                mbh->drives_addr,
                mbh->config_table);
    }

// TODO
//    u32 timestamp = rsptd();
//    srand();

    display_banner();


    set_text_color(COLOR_LIGHT_RED, COLOR_BLACK);

    // TODO: assert(mboot_mag == MULTIBOOT_EAX_MAGIC && "Didn't boot with multiboot, not sure how we got here.");


    // Multiboot Info
    kputs("MBInfo: ");
    printHex(magic);
    kputs(", Mem: ");
    printInt(mbh->mem_lower);
    kputs("-");
    printInt(mbh->mem_upper);
    kputs("B");
    kputs(", Flags:");
    printHex(mbh->flags);
    kputs(", ");
    kputs("\n");


    u32 ticks = timer_ticks();
    kputs("Start Timer");
    //for(int i=0;i<10;i++)
    {
        //kputch('.');
        delay_ms(100);
    }
    kputs("Done.\n");

    kputs("Test took: ");
    printInt(timer_ticks() - ticks);
    kputs("Ticks\n");

    // Generics Test
    my_test(1);
    my_test((long double)2.0);
    my_test(3.f);

    wait_any_key();


    set_text_color(COLOR_LIGHT_GREEN, COLOR_BLACK);

    // TODO: do VBE in real mode
    //if(hasBit(mbh->flags, 11))
    {
        kprintf("VBE: %x,%x,%x,%x,%x\n",
                mbh->vbe_control_info,
                mbh->vbe_mode_info,
                mbh->vbe_mode,
                mbh->vbe_interface_seg,
                mbh->vbe_interface_off,
                mbh->vbe_interface_len);

        kwritef(serial_write_b, "VBE: %x,%x,%x,%x,%x\n",
                mbh->vbe_control_info,
                mbh->vbe_mode_info,
                mbh->vbe_mode,
                mbh->vbe_interface_seg,
                mbh->vbe_interface_off,
                mbh->vbe_interface_len);
    }

    {
        kprintf("MMap:\t%x\nAddr:\t%x\nDrives:\t%xAddr:\t%x\nConfig:\t%x\n",
                mbh->mmap_length,
                mbh->mmap_addr,
                mbh->drives_length,
                mbh->drives_addr,
                mbh->config_table);

        kwritef(serial_write_b,"MMap:\t%x\nAddr:\t%x\nDrives:\t%xAddr:\t%x\nConfig:\t%x\n",
                mbh->mmap_length,
                mbh->mmap_addr,
                mbh->drives_length,
                mbh->drives_addr,
                mbh->config_table);
    }

    //if (mbh->flags & (1 << 3))
    {
        kputs("Found ");
        printInt(mbh->mods_count);
        kputs("module(s).");

        if (mbh->mods_count > 0)
        {
            for (u32 i = 0; i < mbh->mods_count; ++i )
            {
                u32 module_start = *(u32*)(mbh->mods_addr + 8 * i);
                u32 module_end   = *(u32*)(mbh->mods_addr + 8 * i + 4);
                kputs("Module ");
                printInt(i+1);
                kputs(" is at ");
                printHex(module_start);
                kputs(":");
                printHex(module_end);
                kputs("\n");
            }
        }
    }

    //if(hasBit(mbh->flags, 9))
    {
        kputs("Bootloader Name: ");
        kputs((i8*)mbh->boot_loader_name);
        kputs("\n");
    }


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
    kputs("Disk: ");
    printHex_w(ident_data[0]);

    // 01,03,06 - CHS
    kputs(", Cyl:"); printInt(ident_data[1]);
    kputs(", Heads:");  printInt(ident_data[3]);
    kputs(", Sectors:");  printInt(ident_data[6]);

    u32 bytes = chs2bytes(ident_data[1], ident_data[3], ident_data[6]);
    u32 kilobytes = bytes/1024;
    u32 megabytes = bytes/1048576;
    u32 gigabytes = bytes/1073741824;

    kputs("\nStorage Size is ");
    printInt(kilobytes); kputs("KB, ");
    printInt(megabytes); kputs("MB, ");
    printInt(gigabytes); kputs("GB");

    // 10-19 - Serial Number
    kputs("\nSerial: ");
    for(int i=10; i<19; ++i) {
        kputch((ident_data[i] >> 8) & 0xff);
        kputch(ident_data[i] & 0xff);
    }
    // 23-26 Firmware Revision
    kputs("\nFirmware: ");
    for(int i=23; i<26; ++i) {
        kputch((ident_data[i] >> 8) & 0xff);
        kputch(ident_data[i] & 0xff);
    }
    // 27-46 - Model Name
    kputs("\nModel: ");
    for(int i=27; i<46; ++i) {
        kputch((ident_data[i] >> 8) & 0xff);
        kputch(ident_data[i] & 0xff);
    }
    kputch('\n');

    wait_any_key();

    // TODO: ident_data should have a struct type instead
    // 49 - (bit 9) LBA Supported
    if(ident_data[49] & 0x0100)
        kputs("\nLBA Supported!");
    if(ident_data[59] & 0x0100)
        kputs("\nMultiple sector setting is valid!");

    // 60/61 - taken as DWORD => total # LBA28 sectors (if > 0, supports LBA28)
    u32 lba_capacity = (ident_data[61] << 16) + ident_data[60];
    u32 lba_bytes = (lba_capacity/MEGA*SECTOR_BYTES);

    // TODO: write("LBA Cap: %d Sectors, %d MB", lba_capacity, lba_bytes);
    kputs("\nLBA Capacity: ");
    printInt(lba_capacity);
    kputs(" Sectors");

    kputs(", ");
    printInt(lba_bytes);
    kputs(" MB");

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
    kputch('\n');
    for(int i=0; i<10; ++i)
        printHex_w(data[i]);

    // wait
    while((inb(HD_ST_ALT) & 0xc0) != 0x40)
        ;

    // NOTE: QEMU protects block 0 from being written to if img format is unknown or raw
    u16 data2[512];
    ata_pio_read_w(0,1,1,1,data2);
    kputch('\n');
    for(int i=0; i<10; ++i)
        printHex_w(data2[i]);
    kputch('\n');

    // -- END HARD DISK ACCESS TESTING ---


    wait_any_key();

    set_text_color(COLOR_YELLOW, COLOR_BLACK);

    trace("print address test");
    int test = 0;
    kprintf("\nThe address of test is: %x\n", &test);

    wait_any_key();

    // DEBUG::Testing heap code
    print_heap_magic();

    wait_any_key();


    trace("kmalloc");
    u8 *t = kmalloc(4);
    u8 *s = kmalloc(8);
    for(int i=0; i<4; ++i) {
        t[i] = i;
        s[i*2] = i*2;
        s[i*2+1] = i*2+1;
    }
    for(int i=0; i<4; ++i) {
        kprintf("%x : %d : %x : %d : %d", (u32)&t, t[i], (u32)&s, s[i*2], s[i*2+1]);
    }


    wait_any_key();

    // print out some memory
    kputch('\n');
    int array[10] = { 1,1,2,3,5,8,13,21,34 };
    int *p = array;
    for(int i=0; i<10; ++i, ++p) {
        printInt((int)&p);
        kputch(':');
        printInt((int)*p);
        kputs("  ");
        if(i % 7 == 6) kputs("\n");
    }

    trace("print out memory");
    wait_any_key();

    // Test Division By 0
    // need to hide the zero
    //z = 2-1; i = 10 / z; kputch(i);

    //
    kputch('\n');

    // TODO: k_println("{} and {}", a, b);


    wait_any_key();




    // TODO: allow switching "stdout" from direct to serial or both
    rtc_time time = read_rtc();
    kputs("datetime = ");
    //    if(time.century > 0) {
    //        printInt(time.century);
    if(time.year > 100) {
        printInt(time.year);
    } else {
        printInt(time.year + 1970);
        kputs("[1970]");
    }
    kputch('-');
    if(time.month < 10)
        kputch('0');
    printInt(time.month);
    kputch('-');
    if(time.day < 10)
        kputch('0');
    printInt(time.day);
    kputch(' ');
    if(time.hour < 10)
        kputch('0');
    printInt(time.hour);
    kputch(':');
    if(time.minute < 10)
        kputch('0');
    printInt(time.minute);
    kputch(':');
    if(time.second < 10)
        kputch('0');
    printInt(time.second);
    kputch('\n');

    // TODO: fork off shell process
    serial_write("Starting Infinite Loop!\r\n");
    kputs("Starting Infinite Loop!\n");



    kputs("Press SPACE BAR:");
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
    
    
//    // test modeX (non-chain4 allows 400x600 possible)
//    success = init_graph_vga(320,240,0);
//    if(success) 
//    {
//        serial_write("["__DATE__" "__TIME__"] Video Mode Success!\r\n");
//        vga_tests();
//    }

//    void draw_cursor(u16 x, u16 y)
//    {
//
//    }

    i32 x = 0;
    i32 y = 0;
    for (;;) {
        x = mouse_getx();
        y = mouse_gety();
        fillrect(x,y);
        //delay_ms(100);
    }
    
    return 0;
}

