//#define DEBUG 0

#ifndef __SYSTEM_H
#define __SYSTEM_H

#define NULL 0
#define TRUE 1
#define FALSE 0

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
typedef unsigned int	uint32;
typedef unsigned short	uint16;
typedef unsigned char	uint8;
typedef unsigned char	byte;

/* MAIN.C */
extern byte		*memcpy(byte *dest, const byte *src, size_t count);
extern byte 	*memset(byte *dest, byte val, size_t count);
extern uint16	*memsetw(uint16 *dest, uint16 val, size_t count);
extern int 		strlen(const byte *str);
extern byte 	inportb (uint16 _port);
extern void 	outportb (uint16 _port, byte _data);

/* scrn.c */
extern void cls();
extern void putch(char c);
extern void puts(char *str);
extern void settextcolor(byte forecolor, byte backcolor);
extern void init_video();
extern void printInt(int num);

/* GDT.C */
extern void gdt_set_gate(int num, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran);
extern void gdt_install();

/* IDT.C */
extern void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags);
extern void idt_install();

/* This defines what the stack looks like after an ISR was running */
struct regs
{
    unsigned int gs, fs, es, ds;      /* pushed the segs last */
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;  /* pushed by 'pusha' */
    unsigned int int_no, err_code;    /* our 'push byte #' and ecodes do this */
    unsigned int eip, cs, eflags, useresp, ss;   /* pushed by the processor automatically */ 
};

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

/* MM.C */
extern void init_mm();
extern void print_heap_magic();
extern byte *kmalloc(size_t size);

#endif

