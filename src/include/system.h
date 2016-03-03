//#pragma once
#ifndef SYSTEM_H
#define SYSTEM_H

#if !defined(__cplusplus)
#include <stdbool.h> /* C doesn't have booleans by default. */
#endif
// #include <stddef.h>
// #include <float.h>
 #include <limits.h>
// #include <stdtypes.h>
// #include <stdarg.h>
#include <stdint.h>

#include <systemcpp.h>

#ifndef va_start
typedef __builtin_va_list va_list;
#define va_start(ap,last) __builtin_va_start(ap, last)
#define va_end(ap) __builtin_va_end(ap)
#define va_arg(ap,type) __builtin_va_arg(ap,type)
#define va_copy(dest, src) __builtin_va_copy(dest,src)
#endif

#ifndef NULL
#define NULL ((void *)0UL)
#endif

//////////////////////////////////////////////////////////////////
// Macro Playground

/* The token pasting abuse that makes this possible */
#define JOIN_INTERNAL( a, b ) a##b
#define JOIN( a, b ) JOIN_INTERNAL( a, b )
#define REPEAT_10(a) a(0);a(1);a(2);a(3);a(4); a(5);a(6);a(7);a(8);a(9);

//////////////////////////////////////////////////////////////////
// Defined Constants

// make it more clear why we're using static
#define global   static
#define internal static

#define UNUSED_PARAM(x) ((void)(x))
#define UNUSED_VAR(x)   ((void)(x))
#define ALIGN (sizeof(size_t))
#define ONES ((size_t)-1/UCHAR_MAX)
#define HIGHS (ONES * (UCHAR_MAX/2+1))
#define HASZERO(X) (((X)-ONES) & ~(X) & HIGHS)

#define BITOP(A, B, OP) \
((A)[(size_t)(B)/(8*sizeof *(A))] OP (size_t)1<<((size_t)(B)%(8*sizeof *(A))))


#define KILO (1024)            // 2^10
#define MEGA (1024*1024)       // 2^20
#define GIGA (1024*1024*1024)  // 2^30

//////////////////////////////////////////////////////////////////
// Macros

// Volatile - don't move instructions (no optimization)
#define asm          __asm__
#define volatile     __volatile__
#define asm_volatile __asm__ __volatile__
#define pack_struct  __attribute__((packed))

#define sti()   asm_volatile ("sti");
#define cli()   asm_volatile ("cli");
#define nop()   asm_volatile ("nop");
#define iret()  asm_volatile ("iret");
#define pusha() asm_volatile ("pusha");
#define popa()  asm_volatile ("popa");

//------------------

// TODO: improve this imensly with real logging
// TODO: move this into `make debug`
#define _DEBUG_
#ifdef _DEBUG_
#define trace(fmt, ...) kwritef(serial_write_b, fmt, ##__VA_ARGS__)
//#define trace(fmt, ...) kwritef(kputch, fmt, ##__VA_ARGS__)
#else
#define trace(fmt, ...) do {} while(0);
#endif

//#define _DEBUG_INFO_
#ifdef _DEBUG_INFO_
#define trace_info(fmt, ...) trace(fmt, ##__VA_ARGS__)
#else
#define trace_info(fmt, ...) do {} while(0);
#endif


// TODO: maybe move to exist with other print/write (possibly break out multi headers)
#define kprintf(fmt, ...) kwritef(kputch, fmt, ##__VA_ARGS__)

//////////////////////////////////////////////////////////////////
// Types

// Data Types
// TODO: make sure to #ifdef or include in platform-specific header
// TODO: 's' instead of 'i' prefix?
typedef uint8_t    u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef char       i8;
typedef int16_t   i16;
typedef int32_t   i32;
typedef uint8_t    b8;
typedef uint32_t  b32;

typedef float              real32;
typedef double             real64;
typedef const char*         c_str;
typedef char*           c_str_mut;

typedef unsigned long long    u64;
typedef long long             i64;
typedef unsigned long      size_t;

//////////////////////
// Assertions and Errors

#define	CONCAT(x,y)   x ## y
#define	STRINGIFY(x)  #x
extern void kassert_fail(c_str assertion, c_str file, unsigned int line, c_str func, c_str msg);
#define ASSERT(expr,msg) ((void) ((expr) || (kassert_fail(STRINGIFY(expr), __FILE__, __LINE__, __func__, msg), 0)))


//////////////////////////////////////////////////////////////////
// Utilities and Common

extern void* kmemcpy(void* restrict dest, const void* restrict src, size_t n);
extern void* kmemset(void* dest, int c, size_t n);
extern size_t   kmemcmp(const void* vl, const void* vr, size_t n);
extern void* kmemchr(const void* src, int c, size_t n);
extern void* kmemrchr(const void * m, int c, size_t n);
extern void* kmemmove(void * dest, const void * src, size_t n);

extern u8*  kmemcpyb(u8* dest, const u8* src, u32 count);
extern u8*  kmemsetb(u8* dest, u8 val, u32 count);
extern u16* kmemsetw(u16* dest, u16 val, u32 count);
extern u32  kstrlen(c_str str);
extern u32  krand(void);
extern void ksrand(u32 seed);

//////////////////////////////////////////////////////////////////
// Multitasking (Tasks, TSS, etc)

typedef struct {
    // general
    u32 eax, ebx, ecx, edx;
    // special
    u32 esi, edi, esp, ebp, eip;
    // flags
    u32 eflags, cr3;
} task_registers;

typedef struct task {
    task_registers regs;
    struct task *next;
} task;

typedef void(*task_handler)();

/// Initialize the multitasking system and structures
extern void task_init();
/// Kernel interface to switching task
extern void task_preempt();
extern void ktask_create(task*, task_handler, u32, u32*);
/// Kernel impl for switching task
extern void task_switch(task_registers *prev, task_registers *next);

//////////////////////////////////////////////////////////////////
// Real Time & Clock and Timers


// TODO: get a calendar library
// Clock "Real" Time (from battery-backed CMOS)
// second: 0-59, minute: 0-59, day: 0-31, month: 0-12, year: yyyy
typedef struct
{
    u8 second;
    u8 minute;
    u8 hour;
    u8 day;
    u8 month;
    u16 year;
} rtc_time;

extern void timer_install();
extern u32 timer_ticks();
extern u32 timer_seconds();
extern void delay_ticks(i32 ticks);
extern void delay_ms(u32 ms);
extern void delay_s(u32 s);
extern rtc_time read_rtc();

//////////////////////////////////////////////////////////////////
// Filesystem (FS)

/// FS Block -
typedef struct {
    u32 a;
    u8 b;
    i16 add;
} fs_block;


/// FS Master Table
/// - the top level table storing pointers to block paging tables, etc
typedef struct {
} fs_master_table;

//////////////////////////////////////////////////////////////////
// Screen (Terminal, VGA, VESA)

// TODO: const u8
#define COLOR_BLACK         0x00
#define COLOR_BLUE          0x01
#define COLOR_GREEN         0x02
#define COLOR_CYAN          0x03
#define COLOR_RED           0x04
#define COLOR_MAGENTA       0x05
#define COLOR_BROWN         0x06
#define COLOR_LIGHT_GREY    0x07
#define COLOR_DARK_GREY     0x08
#define COLOR_LIGHT_BLUE    0x09
#define COLOR_LIGHT_GREEN   0x0a
#define COLOR_LIGHT_CYAN    0x0b
#define COLOR_LIGHT_RED     0x0c
#define COLOR_LIGHT_MAGENTA 0x0d
#define COLOR_YELLOW        0x0e
#define COLOR_WHITE         0x0f

// TODO: use Apple-style (maybe others) consts ????
enum {
    kColor_Black,
};

extern void cls();
extern u8 kgetch();
extern void kputch(u8 c);
extern void kputs(c_str str);
extern void set_text_color(u8 forecolor, u8 backcolor);
extern void init_video();

// vga
extern u32 init_graph_vga(u32 width, u32 height, b32 chain4);
extern void plot_pixel(u32 x, u32 y, u8 color);
extern void line_fast(u32 x1, u32 y1, u32 x2, u32 y2, u8 color);
extern void polygon(u32 num_vertices,  u32 *vertices, u8 color);
extern void fillrect(u16 xoff, u16 yoff);
extern void vga_tests();


//////////////////////////////////////////////////////////////////
// Device Input/Output

extern u8 inb(u16 _port);
extern void outb(u16 _port, u8 _data);
extern u16 inw(u16 _port);
extern void outw(u16 _port, u16 _data);
extern void print_port(u16 port);

// TODO: move elsewhere??? Inline Asm Funcs
//extern inline void cpuid(int code, u32* a, u32* d);
//extern inline u64 rdtsc();
//extern inline void lidt(void* base, u16 size);
//extern inline void invlpg(void* m);
//extern inline void wrmsr(u32 msr_id, u64 msr_value);
//extern inline u64 rdmsr(u32 msr_id);

//////////////////////////////////////////////////////////////////
// Memory, Page Faults, Page Tables

/// Global Descriptor Table (GDT)
/// - Defines the memory paging table map
extern void gdt_set_gate(u32 num, u32 base, u32 limit, u8 access, u8 gran);
extern void gdt_install();

// Memory Allocation
extern void init_mm(void);
extern void print_heap_magic(void);
extern u8 *kmalloc(u32 size);

//////////////////////////////////////////////////////////////////
// STDOUT and friends

typedef void(*output_writer)(u8 a);

extern void writeInt(output_writer writer, i32 num);
extern void writeUInt(output_writer writer, u32 num);
extern void writeAddr(void* ptr, output_writer writer);
extern void writeHex_b(output_writer writer, u8 num);
extern void writeHex_w(output_writer writer, u16 num);
extern void writeHex(output_writer writer, u32 num);
extern void writeHexDigit(output_writer writer, u8 digit);
extern void writeBinary_b(output_writer writer, u8 num);
extern void writeBinary_w(output_writer writer, u16 num);
extern void writeBinary(output_writer writer, u32 num);
extern void writeChar(output_writer writer, u8 ch);

extern void printInt(i32 num);
extern void printHex(u32 num);
extern void printHex_w(u16 num);
extern void printHex_b(u8 num);
extern void printAddr(void* ptr);
extern void printBinary_b(u8 num);

extern void init_serial();
extern int serial_received();
extern char read_serial();
extern u32 is_transmit_empty();
extern void serial_write_b(u8 a);
extern void serial_write(c_str str);
extern void serial_writeInt(u32 num);
extern void serial_writeHex(u32 num);
extern void serial_writeHex_w(u16 num);
extern void serial_writeHex_b(u8 num);
extern void serial_writeBinary_b(u8 num);

extern void kwrites(output_writer writer, c_str text);
extern void kputs(const char* text);
//extern void kprintf(c_str format, ...);
extern void kwritef(output_writer writer, c_str format, ...);

////////////////////////////////////////////////////////////////////////////
// User Input Devices

extern i32 mouse_getx();
extern i32 mouse_gety();

extern void keyboard_install();
extern void mouse_install();

////////////////////////////////////////////////////////////////////////////
// Disks (Hard Drive, Floppy, CDRom)

#define SECTOR_BYTES       512
#define SECTOR_WORDS       256
#define SECTOR_DWORDS      128     

// could use if wanting more than one controller
#define IDE_PRIMARY        1
#define IDE_SECONDARY      0

// could use if want a programmed way to access ATA registers
#define IDE0_BASE          0x1F0
#define IDE1_BASE          0X170

#define HD0_IRQ            IRQ14
#define HD1_IRQ            IRQ15

#define HD_DATA            0x1f0  // Data port
#define HD_FEAT            0x1f1  // Features (write)
#define HD_ERR             0x1f1  // Error Info (read)
#define HD_SC              0x1f2  // Sector Count
#define HD_SN              0x1f3  // Sector Number (Low Byte of LBA - Partial Disk Sector Address)
#define HD_CL              0x1f4  // Cylinder low-byte (Mid Byte of LBA - Partial Disk Sector Address)
#define HD_CH              0x1f5  // Cylinder high-byte (High Byte of LBA - Partial Disk Sector Address)
#define HD_DH              0x1f6  // Drive select bit, 101DHHHH
#define HD_ST              0x1f7  // Status port (read)
#define HD_CMD             0x1f7  // Command port (write)
#define HD_ST_ALT          0x3f6  // Alternative Status
#define HD_DCR             0x3f6  // Device Control Register (Alternative Status)

#define HD1_DATA           0x170  // Data port
#define HD1_FEAT           0x171  // Features (write)
#define HD1_ERR            0x171  // Error Info (read)
#define HD1_SC             0x172  // Sector Count
#define HD1_SN             0x173  // Sector Number (Low Byte of LBA - Partial Disk Sector Address)
#define HD1_CL             0x174  // Cylinder low-byte (Mid Byte of LBA - Partial Disk Sector Address)
#define HD1_CH             0x175  // Cylinder high-byte (High Byte of LBA - Partial Disk Sector Address)
#define HD1_DH             0x176  // Drive select bit, 101DHHHH
#define HD1_ST             0x177  // Status port (read)
#define HD1_CMD            0x177  // Command port (write)
#define HD1_ST_ALT         0x3f7  // Alternative Status
#define HD1_DCR            0x3f7  // Device Control Register (Alternative Status)

#define HD_DCR_HOB          0x80  // SEt this to read back high-order byte of last LBA48 value sent to IO port.
#define HD_DCR_SRST         0x04  // Software Reset -- set this to reset all ATA drives on a bus, if one is misbehaving.
#define HD_DCR_NIEN         0x02  // Set this to stop the current device from sending interrupts.

// Bits for HD_STATUS
#define HD_ST_ERR           0x01  // Error flag (when set). Send new command to clear it (or nuke with Soft-Reset)
#define HD_ST_IDX           0x02  // ?
#define HD_ST_ECC           0x04  // corrected errors
#define HD_ST_DRQ           0x08  // Set when drive has PIO data to transfer, or is ready to accept PIO data.
#define HD_ST_SK            0x10  // Overlapped Mode Service Request (seek)
#define HD_ST_DFE           0x20  // Drive fault errors (does not set ERR!)
#define HD_ST_RDY           0x40  // Bit is clear when drive is spun down, or after an error. Set otherwise.
#define HD_ST_BSY           0x80  // drive is preparing to accept/send data -- wait until this bit clears. If it never clears, then do a soft-reset. When set other status bits are meaningless.

// Values for HD_CMD
#define HD_CMD_RESTORE      0x10  //
#define HD_CMD_READ         0x20  //
#define HD_CMD_WRITE        0x30  //
#define HD_CMD_VERIFY       0x40  //
#define HD_CMD_FORMAT       0x50  //
#define HD_CMD_INIT         0x60  //
#define HD_CMD_SEEK         0x70  //
#define HD_CMD_DIAGNOSE     0x90  //
#define HD_CMD_SPECIFY      0x91  //
#define HD_CMD_IDENTIFY     0xEC  //

// Bits for HD_ERROR
#define HD_ERR_MARK         0x01  //
#define HD_ERR_TRACK0       0x02  //
#define HD_ERR_ABORT        0x04  //
#define HD_ERR_ID           0x10  //
#define HD_ERR_ECC          0x40  //
#define HD_ERR_BBD          0x80  //

extern void ata_delay400ns(void);
extern void ata_wait_busy();
extern void ata_wait_drq();
extern void ata_wait_ready();
extern int ata_soft_reset(void);
extern int ata_pio_read_w(int controller, int slave, int sn, int sc, u16 *data);
extern int ata_pio_write_w(int controller, int slave, int sn, int sc, u16 *data);
extern int ata_controller_present(int controller);
extern int ata_drive_present(int controller, int drive);
u32 chs2bytes(u16 c, u16 h, u16 s);


//////////////////////////////////////////////////////////////////
// Interrupts

/// IRQ function table (0-7, 8-15)
#define IRQ_COUNT 16

#define IRQ_SYSTEM_TIMER        0
#define IRQ_KEYBOARD_PS2        1
//IRQ 2 – cascaded signals from IRQs 8–15 (any devices configured to use IRQ 2 will actually be using IRQ 9)
#define IRQ_SERIAL_PORT_2_4     3
#define IRQ_SERIAL_PORT_1_3     4
#define IRQ_PARALLEL_PORT_2_3   5
#define IRQ_SOUND_CARD          5
#define IRQ_FLOPPY_DISK         6
#define IRQ_PARALLEL_PORT_1     7
#define IRQ_REAL_TIME_CLOCK     8
#define IRQ_ACPI_CTRL_INTEL     9
//IRQ 10 – The Interrupt is left open for the use of peripherals (open interrupt/available, SCSI or NIC)
//IRQ 11 – The Interrupt is left open for the use of peripherals (open interrupt/available, SCSI or NIC)
#define IRQ_MOUSE_PS2           12
#define IRQ_COPROC_FPU_IPI      13
#define IRQ_ATA_MASTER          14
#define IRQ_ATA_SLAVE           15

/* Defines an IDT entry */
typedef struct pack_struct
{
    u16 base_lo;
    u16 sel;        /* Our kernel segment goes here! */
    u8 always0;     /* This will ALWAYS be set to 0! */
    u8 flags;       /* Set using the above table! */
    u16 base_hi;
} idt_entry;

typedef struct pack_struct
{
    unsigned short limit;
    unsigned int base;
} idt_ptr;


// TODO: we could change the ordering, need to make clear this is ISR specific from ASM code
/// This defines what the stack looks like after an ISR was running
typedef struct
{
    // pushed the segs last
    u32 gs, fs, es, ds;
    // pushed by 'pusha'
    u32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
    // our 'push byte #' and ecodes do this
    u32 int_no, err_code;
    // pushed by the processor automatically
    u32 eip, cs, eflags, useresp, ss;
} isr_stack_state;

typedef void(*isr_handler)(isr_stack_state* r);

/// (in start.s)
extern void idt_load();

/// Interrupt Descriptor Table (IDT)
extern void idt_set_gate(u8 num, u32 base, u16 sel, u8 flags);
extern void idt_install();

// Interrupt Service Routines (ISR)
extern void fault_handler(isr_stack_state *r);
extern void isrs_install();
extern void irq_install_handler(u32 irq, isr_handler handler, c_str name);
extern void irq_uninstall_handler(u32 irq);
extern void irq_remap();
extern void print_irq_counts();

/// Handlers (defined in asm)
// TODO: can we reduce this to single wrapper?
extern isr_handler isr_stubs[128];
extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();


////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Keyboard

// maybe move to it's own include file
// http://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html#ss1.1
/*
 #define SCAN_NULL
 #define SCAN_
 #define SCAN_1 3
 #define SCAN_2
 #define SCAN_3
 #define SCAN_4
 #define SCAN_5
 #define SCAN_6
 #define SCAN_7
 #define SCAN_8
 #define SCAN_9
 #define SCAN_0
 #define SCAN_MINUS
 #define SCAN_EQUAL
 #define SCAN_BACKSPACE
 #define SCAN_TAB
 #define SCAN_Q
 #define SCAN_W
 #define SCAN_E
 #define SCAN_
 #define SCAN_
 #define SCAN_
 #define SCAN_
 #define SCAN_
 #define SCAN_
 #define SCAN_
 #define SCAN_
 #define SCAN_
 #define SCAN_
 #define SCAN_
 #define SCAN_
 #define SCAN_
 #define SCAN_
 #define SCAN_
 #define SCAN_
 #define SCAN_
 */

#define KBDUS_DOWNARROW		137
#define KBDUS_PAGEDOWN		138
#define KBDUS_INSERT		139
#define KBDUS_CONTROL		140
#define KBDUS_LEFTSHIFT		141
#define KBDUS_RIGHTSHIFT	142
#define KBDUS_ALT			143
#define KBDUS_CAPSLOCK		144
#define KBDUS_F1			145
#define KBDUS_F2			146
#define KBDUS_F3			147
#define KBDUS_F4			148
#define KBDUS_F5			149
#define KBDUS_F6			150
#define KBDUS_F7			151
#define KBDUS_F8			152
#define KBDUS_F9			153
#define KBDUS_F10			154
#define KBDUS_F11			155
#define KBDUS_F12			156
#define KBDUS_NUMLOCK		157
#define KBDUS_SCROLLLOCK	158
#define KBDUS_HOME			159
#define KBDUS_DELETE		160
#define KBDUS_PAGEUP		161
#define KBDUS_UPARROW		162
#define KBDUS_RIGHTARROW	163
#define KBDUS_LEFTARROW		164
#define KBDUS_END			165

#define SCAN_US_SPACE 0x39
#define SCAN_US_F2 0x3c

/* KBDUS means US Keyboard Layout. This is a scancode table
 *  used to layout a standard US keyboard. I have left some
 *  comments in to give you an idea of what key is what, even
 *  though I set it's array index to 0. You can change that to
 *  whatever you want using a macro, if you wish! */

typedef u8 kbscan_t;
extern kbscan_t kbdus[128];
extern kbscan_t keyboard_read_next();

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

#endif // SYSTEM_H
