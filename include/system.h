//#define DEBUG 0

#ifndef __SYSTEM_H
#define __SYSTEM_H

//#define _DEBUG_

// Defined Constants
#define NULL 0
#define TRUE 1
#define true 1
#define FALSE 0
#define false 0

#define KILO (1024)
#define MEGA (1024*1024)
#define GIGA (1024*1024*1024)

// Defined Macros
#define sti() __asm__ __volatile__ ("sti");
#define cli() __asm__ __volatile__ ("cli");
#define nop() __asm__ __volatile__ ("nop");
#define iret() __asm__ __volatile__ ("iret");
#define pusha() __asm__ __volatile__ ("pusha");
#define popa() __asm__ __volatile__ ("popa");

#ifdef _DEBUG_
#define trace(x) puts(x)
#else
#define trace(x)
#endif

/*
 * Types: these types are used to clarify what is meant when using them
 * instead of having byte for byte. Also for more concise code.
 * ::NOTE:: These are comiler and architecture specific. The c standard
 * actually defines int >= short >= char = 8-bits, and thus you can't be
 * certain that unsigned int is really 32-bits. However, since we know 
 * we are using 32-bit-x86 (IA-32) and gcc/nasm supporting 32-bits, we 
 * know that these are the correct typedefs.
 */
typedef unsigned int 	size_t;
typedef unsigned int	uint;
typedef unsigned int	uint32;
typedef unsigned short	uint16;
typedef unsigned char	uint8;
typedef uint8			byte;
typedef uint16			word;
typedef uint32			dword;

typedef unsigned int	bool;
typedef bool			boolean;

/* This defines what the stack looks like after an ISR was running */
struct regs
{
    unsigned int gs, fs, es, ds;      /* pushed the segs last */
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;  /* pushed by 'pusha' */
    unsigned int int_no, err_code;    /* our 'push byte #' and ecodes do this */
    unsigned int eip, cs, eflags, useresp, ss;   /* pushed by the processor automatically */ 
};


/* MAIN.C */
extern byte		*memcpy(byte *dest, const byte *src, size_t count);
extern byte 	*memset(byte *dest, byte val, size_t count);
extern uint16	*memsetw(uint16 *dest, uint16 val, size_t count);
extern int 		strlen(const byte *str);

/* IO.C */
extern byte 	inb (uint16 _port);
extern void 	outb (uint16 _port, byte _data);
extern word 	inw (uint16 _port);
extern void 	outw (uint16 _port, word _data);
extern void 	print_port(uint16 port);

/* scrn.c */
extern void cls();
extern char getch();
extern void putch(char c);
extern void puts(char *str);
extern void settextcolor(byte forecolor, byte backcolor);
extern void init_video();
extern void printInt(int num);
extern void printHex(byte b);
extern void printHex_w(word w);
extern void printHexDigit(byte digit);
extern void printBin_b(byte num);
extern void printBin_w(word num);

/* GDT.C */
extern void gdt_set_gate(int num, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran);
extern void gdt_install();

/* IDT.C */
extern void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags);
extern void idt_install();


/* ISRS.C */
extern void fault_handler(struct regs *r);
extern void isrs_install();

/* IRQ.C */
extern void irq_install_handler(int irq, void (*handler)(struct regs *r));
extern void irq_uninstall_handler(int irq);
extern void irq_install();

/* TIMER.C */
extern void timer_wait(int ticks);
extern void timer_install();
extern uint32 time_ticks();
extern uint32 time_s();

/* KEYBOARD.C */
extern void keyboard_install();

// MM.C
extern void init_mm(void);
extern void print_heap_magic(void);
extern byte *kmalloc(size_t size);

// HD.C
extern void ata_delay400ns(void);
extern void ata_wait_busy();
extern void ata_wait_drq();
extern void ata_wait_ready();
extern int ata_soft_reset(void);
extern int ata_pio_read_w(int controller, int slave, int sn, int sc, word *data);
extern int ata_pio_write_w(int controller, int slave, int sn, int sc, word *data);
extern int ata_controller_present(int controller);
extern int ata_drive_present(int controller, int drive);

#endif

