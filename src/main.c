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
    // #error "This tutorial needs to be compiled with a i386-elf compiler"
#endif

#include <system.h>
#include <systemcpp.h>
#include <multiboot.h>

//////////////////////////////

output_writer TRACE_WRITER = serial_write_b;

u32 initial_esp;

/////////////////////////////

void kassert_fail(c_str assertion, c_str file, unsigned int line, c_str func, c_str msg)
{
    for(int i=0; i<2; ++i) {
        output_writer writer = i == 0 ? serial_write_b : kputch;
        kwritef(writer, "[ASSERT]: %d: %s: %s: %s\n[%s]", line, file, func, msg, assertion);
    }
}

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

void* kmemchr(const void * src, int c, size_t n)
{
    const unsigned char * s = src;
    c = (unsigned char)c;
    // TODO: clarify code

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

void wait_any_key() {
    set_text_color(COLOR_LIGHT_MAGENTA, COLOR_BLACK);

    // TODO: clear_key_buffer();
    kputs("Clearing Key Buffer\n");
    while(keyboard_read_next()) {
        kputch('.');
    }

    kputs("Waiting for Key\n");
    kputs("Press Any Key!");
    kgetch();

    set_text_color(COLOR_WHITE, COLOR_BLACK);

    kputs("\n");
    kputs("key pressed\n");
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


//#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))

internal vbe_controller_info *vbe_ctrl;
internal vbe_mode_info *vbe;
internal uint8_t *mem;

// 24bpp
internal void draw_pixel(u32 x, u32 y, u32 color)
{
    u32 pos = y * vbe->LinBytesPerScanLine + x * (vbe->bpp / 8);
    // Assuming 24-bit color mode
    mem[pos] = color & 0xFF;
    mem[pos + 1] = (color >> 8) & 0xFF;
    mem[pos + 2] = (color >> 16) & 0xFF;
    //trace("draw pixel {%d, %d} [%d] w/color %x\n    ", x, y, pos, color);
}

internal void draw_rectangle(u32 x, u32 y, u32 width, u32 height, uint32_t color)
{
    trace("draw rect {%d, %d, %d, %d} w/color %x\n", x, y, width, height, color);
    for (u32 i = x; i < x + width; ++i)
    {
        for (u32 j = y; j < y + height; ++j)
        {
            draw_pixel(i, j, color);
        }
    }
}

// convert [seg]:[off] u32=u16:u16 into linear
static void* linear_addr(segoff p)
{
    return (void*)((p.seg << 4) + p.off);
}

internal void test_harddisk()
{
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
    kprintf("Disk: %x, Cyl:%d, Head:%d, Sec:%d\n",
            ident_data[0],
            ident_data[1],
            ident_data[3],
            ident_data[6]);

    u32 bytes = chs2bytes(ident_data[1], ident_data[3], ident_data[6]);
    u32 kilobytes = bytes/1024;
    u32 megabytes = bytes/1048576;
    u32 gigabytes = bytes/1073741824;

    kprintf("Storage Size is %dKB, %dMB, %dGB\n",
            kilobytes, megabytes, gigabytes);

    // 10-19 - Serial Number
    kputs("Serial: ");
    for(int i=10; i<19; ++i) {
        kputch((ident_data[i] >> 8) & 0xff);
        kputch(ident_data[i] & 0xff);
    }
    kputs("\n");

    // 23-26 Firmware Revision
    kputs("Firmware: ");
    for(int i=23; i<26; ++i) {
        kputch((ident_data[i] >> 8) & 0xff);
        kputch(ident_data[i] & 0xff);
    }
    kputs("\n");

    // 27-46 - Model Name
    kputs("Model: ");
    for(int i=27; i<46; ++i) {
        kputch((ident_data[i] >> 8) & 0xff);
        kputch(ident_data[i] & 0xff);
    }
    kputs("\n");

    wait_any_key();

    // TODO: ident_data should have a struct type instead
    // 49 - (bit 9) LBA Supported
    if(ident_data[49] & 0x0100)
        kputs("LBA Supported!\n");
    if(ident_data[59] & 0x0100)
        kputs("Multiple sector setting is valid!\n");

    // 60/61 - taken as DWORD => total # LBA28 sectors (if > 0, supports LBA28)
    u32 lba_capacity = (ident_data[61] << 16) + ident_data[60];
    u32 lba_bytes = (lba_capacity/MEGA*SECTOR_BYTES);

    kprintf("LBA Capacity: %d sectors, %dMB\n", lba_capacity, lba_bytes);

    if(ata_controller_present(0)){
        trace_info("\nController 0 EXISTS");
    } else {
        trace_info("\nController 0 NOT EXIST");
    }

    if(ata_controller_present(1)){
        trace_info(" Controller 1 EXISTS");
    } else {
        trace_info(" Controller 1 NOT EXIST");
    }

    //ata_soft_reset();

    if(ata_drive_present(0, 0)){
        trace_info("\nPri Drive 0 EXISTS");
    } else {
        trace_info("\nPri Drive 0 NOT EXIST");
    }

    if(ata_drive_present(0, 1)){
        trace_info(" Pri Drive 1 EXISTS");
    } else {
        trace_info(" Pri Drive 1 NOT EXIST");
    }

    if(ata_drive_present(1, 0)){
        trace_info("\nSec Drive 0 EXISTS");
    } else {
        trace_info("\nSec Drive 0 NOT EXIST");
    }

    if(ata_drive_present(1,1)){
        trace_info(" Sec Drive 1 EXISTS");
    } else {
        trace_info(" Sec Drive 1 NOT EXIST");
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
}

// NASM assembly boot loader calls this method
u32 kmain(multiboot_info* mbh, u32 magic, u32 initial_stack)
{
    kserialf( "\n\n\n\n");
    for(int i=0; i<10; ++i)
        kserialf( "=============================================================\n");
    kserialf( "\n\n\n\n");

    initial_esp = initial_stack;

    gdt_install();
    idt_install();
    timer_install();
    ps2_install();

    init_serial();
    init_mm();
    //TODO: init_tasking();
    //TODO: init_paging();

    init_video();

    sti();

    delay_ms(500);

    trace("=============================================================\n");
    trace("test double parsing: %f\n", 12349434.323423499);
    trace("test double parsing: %f\n", 9582983498293849283984.133);

    // TODO: kdebugf (prints to both stdout and serial port)
    u32 a = addi(1,2);
    u32 b = addl(1,2);
    u64 c = addll(1,2);
    trace("Test: calling %d, %d, %d\n", a, b, c);

    // TODO: create text mode first if no VBE
    if(BIT(mbh->flags, 11))
    {

        for(int i=0; i<4; ++i)
            trace("=============================================================\n");

        for(int i=0; i<2; ++i)
        {
            //output_writer writer = (i == 0) ? serial_write_b : kputch;
            TRACE_WRITER = (i == 0) ? kputch : serial_write_b;

            trace("VBE: %x,%x,%x,%x,%x,%x\n",
                    mbh->vbe_controller_info,
                    mbh->vbe_mode_info,
                    mbh->vbe_mode,
                    mbh->vbe_interface_seg,
                    mbh->vbe_interface_off,
                    mbh->vbe_interface_len);
        }

        vbe_ctrl = (vbe_controller_info*)mbh->vbe_controller_info;

        trace("\n");
        trace("vbe_control_info: %x\n", (u32)&mbh->vbe_controller_info);
        trace("vbe buff [addr]: %x\n", (u32)&vbe_ctrl->buff[0]);
        trace("\n");

        trace("signature: %s [%x] addr: %x\nver: %x\noem: %s\ncaps: %x\n",
                &vbe_ctrl->signature, (u32)vbe_ctrl->signature, vbe_ctrl->signature,
                vbe_ctrl->version,
                (c_str*)linear_addr(vbe_ctrl->oem),
                vbe_ctrl->caps);

        trace("oem vendor name: %s\n", (c_str*)linear_addr(vbe_ctrl->oem_vendor_name));
        trace("oem product name: %s\n", (c_str*)linear_addr(vbe_ctrl->oem_product_name));
        trace("oem product rev: %s\n", (c_str*)linear_addr(vbe_ctrl->oem_product_revision));

        trace("list of modes: [%x]\n", vbe_ctrl->mode_list.segoff);

        //u32* mode = linear_addr(vbe_ctrl->mode_list) - 34;

        // TODO: should really just not bother with the multiboot VESA, unless we're only using QEMU/BOCHS
        // TODO: we could 
        u16* mode = (u16*)&vbe_ctrl->reserved[0];
        for(u32 i=0; 0xFFFF != *mode; ++mode, ++i) {
            trace("\t[%x]: %x", mode, *mode);
            if(i % 8 == 7)
                trace("\n");
            if(i > 512) break;
        }
        trace("\n");

        for(int i=0; i<4; ++i)
            trace("===================================================================\n");

        // https://codereview.stackexchange.com/questions/108168/vbe-bdf-font-rendering
        vbe = (vbe_mode_info*)mbh->vbe_mode_info;
        trace("attributes: %u\nwinA: %u\nwinB: %u\ngranularity: %u\nwinsize: %u\nsegmentA: %x\nsegmentB: %x\nwinFuncPtr: %x\npitch: %u\nXres: %u\nYres: %u\nWchar: %u\nYchar: %u\nplanes: %u\nbpp: %u\nbanks: %u\nmemory_model: %u\nbank_size: %u\nimage_pages: %u\nreserved0: %u\nred_mask_size: %u\nred_position: %u\ngreen_mask_size: %u\ngreen_position: %u\nblue_mask_size: %u\nblue_position: %u\nrsv_mask: %u\nrsv_position: %u\ndirectcolor_attributes: %u\nphysbase: %x\nreserved1: %u\nreserved2: %u\nLinBytesPerScanLine: %u\nBnkNumberofImagePages: %u\nLinNumberofImagePages: %u\nLinRedMaskSize: %u\nLinRedFieldPosition: %u\nLinGreenMaskSize: %u\nLinGreenFieldPosition: %u\nLinBlueMaskSize: %u\nLinBlueMaskPosition: %u\nLinRsvdMaskSize: %u\nLinRsvdFieldPosition: %u\nMaxPixelClock: %u\nReserved: %u\n",
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


        mem = (u8*)vbe->physbase;
        //kmemset((u32*)vbe->physbase, 0xff2222, 2^16);

        uint16_t uses_lfb = BIT(mbh->vbe_mode, 14);
        if (uses_lfb)
            trace("uses LFB\n");
        int32_t colors[] = {
            0xFFFFFF, 0xC0C0C0, 0x808080,
            0x000000, 0xFF0000, 0x800000,
            0xFFFF00, 0x808000, 0x00FF00,
            0x008000, 0x00FFFF, 0x008080,
            0x0000FF, 0x000080, 0xFF00FF,
            0x800080
        };

        int idx = 0;
        int chunk_size = 8;
        int xchunk = (vbe->Xres / chunk_size);
        int ychunk = (vbe->Yres / chunk_size);


        trace("chunk_size = %d, xchunk = %d, ychunk = %d\n", chunk_size, xchunk, ychunk);

        for (int i = 0; i < chunk_size; ++i)
        {
            for (int j = 0; j < chunk_size; ++j)
            {
                draw_rectangle(j * xchunk, i * ychunk, xchunk, ychunk, colors[idx]);
                if (++idx == 16)
                    idx = 0;
            }
        }
    }

// TODO
//    u32 timestamp = rsptd();
//    srand();

    display_banner();


    set_text_color(COLOR_LIGHT_RED, COLOR_BLACK);

    // TODO: assert(mboot_mag == MULTIBOOT_EAX_MAGIC && "Didn't boot with multiboot, not sure how we got here.");

    trace("\n");

    u32 ticks = timer_ticks();
    trace("Start Delay.\n");
    for(int i=0;i<10;i++)
    {
        delay_ms(100);
    }
    trace("Done.\n");

    trace("Test took: %d ticks\n", timer_ticks() - ticks);

    // Generics Test
    my_test(1);
    my_test((long double)2.0);
    my_test(3.f);

    wait_any_key();

    set_text_color(COLOR_LIGHT_GREEN, COLOR_BLACK);


    // TODO: do VBE in real mode
    trace("\n\n");
    for(int i=0; i<2; ++i)
    {
        //output_writer writer = (i == 0) ? serial_write_b : kputch;
        TRACE_WRITER = (i == 0) ? kputch : serial_write_b;

        // Multiboot Info
        trace("MBInfo: \t%x [%x], Mem: %d - %d B, Flags: %b\n",
                magic, &mbh, mbh->mem_lower, mbh->mem_upper, mbh->flags);

        trace("MMap:   \t%x\n", mbh->mmap_length);
        trace("Addr:   \t%x\n", mbh->mmap_addr);
        trace("Drives: \t%x\n", mbh->drives_length);
        trace("Addr:   \t%x\n", mbh->drives_addr);
        trace("Config: \t%x\n", mbh->config_table);

        trace("Found %d modules.\n", mbh->mods_count);
        if (mbh->mods_count > 0)
        {
            for (u32 i = 0; i < mbh->mods_count; ++i )
            {
                u32 module_start = *(u32*)(mbh->mods_addr + 8 * i);
                u32 module_end   = *(u32*)(mbh->mods_addr + 8 * i + 4);
                trace("Module %d is at %x : %x\n",
                      i+1, module_start, module_end);
            }
        }

        trace("Bootloader Name: %s\n", (i8*)mbh->boot_loader_name);
    }
    wait_any_key();
    set_text_color(COLOR_MAGENTA, COLOR_BLACK);

    test_harddisk();

    wait_any_key();
    set_text_color(COLOR_YELLOW, COLOR_BLACK);

    trace("print address test\n");
    int test = 0;
    kprintf("\nThe address of test is: %x\n", &test);

    wait_any_key();

        //--------------------------------------------

    trace("kmalloc\n");
    u8 *t = kmalloc_b(4);
    u8 *s = kmalloc_b(8);
    for(int i=0; i<4; ++i) {
        t[i] = i;
        s[i*2] = i*2;
        s[i*2+1] = i*2+1;
    }
    for(int i=0; i<4; ++i) {
        kprintf("%x : %d : %x : %d : %d\n", (u32)&t, t[i], (u32)&s, s[i*2], s[i*2+1]);
    }

    // DEBUG::Testing heap code
    print_heap_magic();
    u8* testMalloc = kmalloc_b(20);
    testMalloc[0] = 'S';
    testMalloc[1] = 't';
    testMalloc[2] = 'e';
    testMalloc[3] = 'v';
    testMalloc[4] = 'e';
    print_heap_bytes(128);
    trace("\n");
    print_blocks_avail();

    wait_any_key();

    //--------------------------------------------

    // print out some stack memory
    kputch('\n');
    int array[10] = { 1,1,2,3,5,8,13,21,34 };
    int *p = array;
    for(int i=0; i<10; ++i, ++p) {
        kprintf("%d:%d , ", &p, *p);
        if(i % 4 == 3)
            kputs("\n");
    }
    kputs("\n");

    wait_any_key();

    //--------------------------------------------

    // TODO: allow switching "stdout" from direct to serial or both
    rtc_time time = read_rtc();
    kprintf("datetime = %d-%d-%d %d:%d:%d\n",
            time.year, time.month, time.day,
            time.hour, time.minute, time.second);
    


    // TODO: fork off shell process
    trace("Starting Infinite Loop!\r\n");
    kputs("Starting Infinite Loop!\n");

    kputs("Press SPACE BAR:");
    for(u8 ch = 0; ch != SCAN_US_SPACE; ch = keyboard_read_next()) {
        trace("still waiting for SPACE KEY PRESS!\n");
        delay_ms(100);
    }

    wait_any_key();

    //--------------------------------------------

    init_page_directory();
    trace("[main] set up page directory!");
    wait_any_key();

    //--------------------------------------------

    // TODO: fix this
    initTasking();
    trace("[main] before preempt\n");
    k_preempt();
    trace("[main] back after preempt\n");
    wait_any_key();

    //--------------------------------------------

    // TEST Setup VGA Graphics mode
    //set_video_mode(video_mode_13h);
    //set_video_mode(video_mode_13h)
    u32 success = init_graph_vga(320,200,1);
    if(success)
    {
        trace("["__DATE__" "__TIME__"] Video Mode Success!\r\n");
        vga_tests();
    }

//    // test modeX (non-chain4 allows 400x600 possible)
//    success = init_graph_vga(320,240,0);
//    if(success) 
//    {
//        trace("["__DATE__" "__TIME__"] Video Mode Success!\r\n");
//        vga_tests();
//    }


    // TODO: fork this into another process
    static u16 x = 0;
    static u16 y = 0;
    static u16 screen_width = 320;
    static u16 screen_height = 200;
    static u8 colorIndex = 0;

    for (;;) {
        x = CLAMP(mouse_get_x(), 0, screen_width);
        y = CLAMP(mouse_get_y(), 0, screen_height);

        u32 buttons = mouse_get_buttons();
        //u8 scroll = mouse_gety();
        if(buttons & MOUSE_SCROLL_UP)
            ++colorIndex;
        else if(buttons & MOUSE_SCROLL_DOWN)
            --colorIndex;

        fillrect(x, y, colorIndex);

        trace("Running Process MAIN!\n");
        delay_ms(100);

        k_preempt();
    }


//    // TODO: once the previous is forked, this should behave okay
//    // Test Division By 0
//    // need to hide the zero
//    trace("trying divide by zero");
//    int i,z;
//    z = 2-2; i = 10 / z; kputch(i);
//
//
//    trace("exiting main");

    return 0;
}

