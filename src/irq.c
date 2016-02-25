#include <system.h>

// Interrupts & CPU Exception Handling
// - http://wiki.osdev.org/Interrupts
// - http://wiki.osdev.org/Exceptions
// - https://en.wikipedia.org/wiki/Interrupt_descriptor_table

/* These are own ISRs that point to our special IRQ handler
*  instead of the regular 'fault_handler' function */
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

typedef void(*irq_handler_t)(isr_stack_state* r);
/// IRQ function table (0-7, 8-15)
irq_handler_t irq_routines[16] =
{
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

/// This installs a custom IRQ handler for the given IRQ
void irq_install_handler(u32 irq, irq_handler_t handler)
{
    irq_routines[irq] = handler;
}

/* This clears the handler for a given IRQ */
void irq_uninstall_handler(u32 irq)
{
    irq_routines[irq] = 0;
}

/// Remap IRQs 0-7 since they are mapped to IDT entries 8-15 by default
/// IRQ 0 is mapped to IDT entry 8 is Double Fault
/// (only an issue in protected mode: )
void irq_remap(void)
{
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0);
    outb(0xA1, 0x0);

    // reset PIC's
    outb(0x21, 0x20);
    outb(0xA1, 0x20);
}

/// Map IRQ to USER area of the IDT
void irq_install()
{
    irq_remap();

    // mapping IRQ handlers to the USER ISRs
    idt_set_gate(32, (u32)irq0, 0x08, 0x8E);
    idt_set_gate(33, (u32)irq1, 0x08, 0x8E);
    idt_set_gate(34, (u32)irq2, 0x08, 0x8E);
    idt_set_gate(35, (u32)irq3, 0x08, 0x8E);
    idt_set_gate(36, (u32)irq4, 0x08, 0x8E);
    idt_set_gate(37, (u32)irq5, 0x08, 0x8E);
    idt_set_gate(38, (u32)irq6, 0x08, 0x8E);
    idt_set_gate(39, (u32)irq7, 0x08, 0x8E);
    idt_set_gate(40, (u32)irq8, 0x08, 0x8E);
    idt_set_gate(41, (u32)irq9, 0x08, 0x8E);
    idt_set_gate(42, (u32)irq10, 0x08, 0x8E);
    idt_set_gate(43, (u32)irq11, 0x08, 0x8E);
    idt_set_gate(44, (u32)irq12, 0x08, 0x8E);
    idt_set_gate(45, (u32)irq13, 0x08, 0x8E);
    idt_set_gate(46, (u32)irq14, 0x08, 0x8E);
    idt_set_gate(47, (u32)irq15, 0x08, 0x8E);
}

/* Each of the IRQ ISRs point to this function, rather than
*  the 'fault_handler' in 'isrs.c'. The IRQ Controllers need
*  to be told when you are done servicing them, so you need
*  to send them an "End of Interrupt" command (0x20). There
*  are two 8259 chips: The first exists at 0x20, the second
*  exists at 0xA0. If the second controller (an IRQ from 8 to
*  15) gets an interrupt, you need to acknowledge the
*  interrupt at BOTH controllers, otherwise, you only send
*  an EOI command to the first controller. If you don't send
*  an EOI, you won't raise any more IRQs */

/// IRQ handler - called from assembly
void irq_handler(isr_stack_state* r)
{
    /* This is a blank function pointer */
    void (*handler)(isr_stack_state* r);

	/* Find out if we have a custom handler to run for this
	*  IRQ, and then finally, run it */
    handler = irq_routines[r->int_no - 32];
    if (handler)
    {
        handler(r);
    }      

	/* If the IDT entry that was invoked was greater than 40
	*  (meaning IRQ8 - 15), then we need to send an EOI to
	*  the slave controller */
    if (r->int_no >= 40)
    {
        outb(0xA0, 0x20);
    }

	/* In either case, we need to send an EOI to the master
	*  interrupt controller too */
    outb(0x20, 0x20);
}
