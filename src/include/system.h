#pragma once

////////////////////////////////////////////////////////////////
// Defined Constants

#define NULL 0
#define TRUE 1
#define true 1
#define FALSE 0
#define false 0

// TODO: const u32
#define KILO (1024)            // 2^10
#define MEGA (1024*1024)       // 2^20
#define GIGA (1024*1024*1024)  // 2^30

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
#define COLOR_LIGHT_BROWN   0x0e
#define COLOR_WHITE         0x0f
enum class {
    Black
};

////////////////////////////////////////////////////////////////
// Defined Macros

#define sti()   __asm__ __volatile__ ("sti");
#define cli()   __asm__ __volatile__ ("cli");
#define nop()   __asm__ __volatile__ ("nop");
#define iret()  __asm__ __volatile__ ("iret");
#define pusha() __asm__ __volatile__ ("pusha");
#define popa()  __asm__ __volatile__ ("popa");

#ifdef _DEBUG_
#define trace(x) puts(x)
#else
#define trace(x)
#endif

//////////////////////////////////////////////////////////////////

// Jai-inspired types
// TODO: make sure to #ifdef or include in platform-specific header
typedef unsigned char   u8;
typedef unsigned short  u16;
typedef unsigned int    u32;
typedef char            i8;
typedef short           i16;
typedef int             i32;
typedef unsigned int    b32;
typedef unsigned char   b8;
typedef float           real32;
typedef double          real64;
typedef const char*     c_str;

//////////////////////////////////////////////////////////////
// Structs

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


// Clock "Real" Time (from battery-backed CMOS)
typedef struct 
{
	u8 second;
	u8 minute;
	u8 hour;
	u8 day;
	u8 month;
	u32 year;
} rtc_time;

/////////////////////////////////////
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
// Func Decls

/* MAIN.C */
extern u8*  memcpy(u8* dest, const u8* src, u32 count);
extern u8*  memset(u8* dest, u8 val, u32 count);
extern u16* memsetw(u16* dest, u16 val, u32 count);
extern u32  strlen(c_str str);

extern u32  rand(void);
extern void srand(u32 seed);

/* IO.C */
extern u8   inb (u16 _port);
extern void outb (u16 _port, u8 _data);
extern u16  inw (u16 _port);
extern void outw (u16 _port, u16 _data);
extern void print_port(u16 port);

extern void init_serial();
extern int serial_received();
extern char read_serial();
extern int is_transmit_empty();
extern void write_serial_b(u8 a);
extern void write_serial(char* str);
/* scrn.c */
extern void cls();
extern char getch();
extern void putch(u8 c);
extern void puts(const char* str);
extern void settextcolor(u8 forecolor, u8 backcolor);
extern void init_video();
extern void printInt(int num);
extern void printHex_b(u8 b);
extern void printHex_w(u16 w);
extern void printHex(u32 w);
extern void printHexDigit(u8 digit);
extern void printBinary_b(u8 num);
extern void printBinary_w(u16 num);
extern void printBinary(u32 num);

/// Global Descriptor Table (GDT)
/// - Defines the memory paging table map
extern void gdt_set_gate(u32 num, u32 base, u32 limit, u8 access, u8 gran);
extern void gdt_install();

/// Interrupt Descriptor Table (IDT)
extern void idt_set_gate(u8 num, u32 base, u16 sel, u8 flags);
extern void idt_install();


// Interrupt Service Routines (ISR)
extern void fault_handler(isr_stack_state *r);
extern void isrs_install();

/* IRQ.C */
extern void irq_install_handler(u32 irq, void (*handler)(isr_stack_state *r));
extern void irq_uninstall_handler(u32 irq);
extern void irq_install();

/* TIMER.C */
extern void timer_install();
extern u32 timer_ticks();
extern u32 timer_seconds();
extern void delay_ticks(i32 ticks);
extern void delay_ms(u32 ms);
extern void delay_s(u32 s);

// extern u32 get_update_in_progress_flag();
// extern u8 get_RTC_register(u32 reg);
extern rtc_time read_rtc();

/* KEYBOARD.C */
extern void keyboard_install();

// MM.C
extern void init_mm(void);
extern void print_heap_magic(void);
extern u8 *kmalloc(u32 size);

// HD.C
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

// vga 
extern u32 init_graph_vga(u32 width, u32 height, b8 chain4);
extern void plot_pixel(u32 x, u32 y, u8 color);
extern void line_fast(u32 x1, u32 y1, u32 x2, u32 y2, u8 color);
extern void polygon(u32 num_vertices,  u32 *vertices, u8 color);

extern void vga_tests();


//#endif

