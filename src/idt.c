#include <system.h>

/* Defines an IDT entry */
typedef struct
{
    unsigned short base_lo;
    unsigned short sel;        /* Our kernel segment goes here! */
    unsigned char always0;     /* This will ALWAYS be set to 0! */
    unsigned char flags;       /* Set using the above table! */
    unsigned short base_hi;
} __attribute__((packed)) idt_entry;

typedef struct
{
    unsigned short limit;
    unsigned int base;
} __attribute__((packed)) idt_ptr;

/* Declare an IDT of 256 entries. Although we will only use the
*  first 32 entries in this tutorial, the rest exists as a bit
*  of a trap. If any undefined IDT entry is hit, it normally
*  will cause an "Unhandled Interrupt" exception. Any descriptor
*  for which the 'presence' bit is cleared (0) will generate an
*  "Unhandled Interrupt" exception */
idt_entry idt[256];
idt_ptr idtp;

/* This exists in 'start.asm', and is used to load our IDT */
extern void idt_load();

/// Add IDT entry
void idt_set_gate(u8 num, u32 base, u16 sel, u8 flags)
{
	// The interrupt routine's base address
    idt[num].base_lo = (base & 0xFFFF);
    idt[num].base_hi = (base >> 16) & 0xFFFF;
    idt[num].sel = sel;
    idt[num].always0 = 0;
    idt[num].flags = flags;
}

// Install IDT
void idt_install()
{
    // Sets the special IDT pointer up, just like in 'gdt.c'
    idtp.limit = (sizeof (idt_entry) * 256) - 1;
    idtp.base = (unsigned int)&idt;

    // Clear out the entire IDT, initializing it to zeros
    memset((u8 *)&idt, 0, sizeof(idt_entry) * 256);

    // Add any new ISRs to the IDT here using idt_set_gate

    // Points the processor's internal register to the new IDT
    idt_load();
}

