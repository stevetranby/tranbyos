#include "include/system.h"

// Interrupts & CPU Exception Handling
// - http://wiki.osdev.org/Interrupts
// - http://wiki.osdev.org/Exceptions
// - https://en.wikipedia.org/wiki/Interrupt_descriptor_table

void disable_irq(int irq);
void disable_irq_nosync(int irq);
void enable_irq(int irq);

////////////////////////////////////////////////////////////////////////////////////
/// IDT

#define MAX_INTERRUPTS 256

// Store 256 interrupt entries, 32 ISR, 16 IRQ, rest "unhandled interrupt" exception
idt_entry   idt[MAX_INTERRUPTS];
idt_ptr     idtp;

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
    idtp.limit = sizeof(idt_entry) * 256 - 1;
    idtp.base = (u32)&idt;

    // Clear out the entire IDT, initializing it to zeros
    kmemsetb((u8*)&idt, 0, sizeof(idt));

    // Add any new ISRs to the IDT here using idt_set_gate

    // Points the processor's internal register to the new IDT
    idt_load();
}

//////////////////////////////////////////////////////////////////
// ISRS

/// Setup the IDT with to map interrupts to correct handler
void isrs_install()
{
    // TODO: use an "unused" stub/handler for those not used??
    // TODO: code generator or move this into NASM where we have repeat macro
    idt_set_gate(0, (u32)isr0, 0x08, 0x8E);
    idt_set_gate(1, (u32)isr1, 0x08, 0x8E);
    idt_set_gate(2, (u32)isr2, 0x08, 0x8E);
    idt_set_gate(3, (u32)isr3, 0x08, 0x8E);
    idt_set_gate(4, (u32)isr4, 0x08, 0x8E);
    idt_set_gate(5, (u32)isr5, 0x08, 0x8E);
    idt_set_gate(6, (u32)isr6, 0x08, 0x8E);
    idt_set_gate(7, (u32)isr7, 0x08, 0x8E);
    idt_set_gate(8, (u32)isr8, 0x08, 0x8E);
    idt_set_gate(9, (u32)isr9, 0x08, 0x8E);
    idt_set_gate(10, (u32)isr10, 0x08, 0x8E);
    idt_set_gate(11, (u32)isr11, 0x08, 0x8E);
    idt_set_gate(12, (u32)isr12, 0x08, 0x8E);
    idt_set_gate(13, (u32)isr13, 0x08, 0x8E);
    idt_set_gate(14, (u32)isr14, 0x08, 0x8E);
    idt_set_gate(15, (u32)isr15, 0x08, 0x8E);
    idt_set_gate(16, (u32)isr16, 0x08, 0x8E);
    idt_set_gate(17, (u32)isr17, 0x08, 0x8E);
    idt_set_gate(18, (u32)isr18, 0x08, 0x8E);
    idt_set_gate(19, (u32)isr19, 0x08, 0x8E);
    idt_set_gate(20, (u32)isr20, 0x08, 0x8E);
    idt_set_gate(21, (u32)isr21, 0x08, 0x8E);
    idt_set_gate(22, (u32)isr22, 0x08, 0x8E);
    idt_set_gate(23, (u32)isr23, 0x08, 0x8E);
    idt_set_gate(24, (u32)isr24, 0x08, 0x8E);
    idt_set_gate(25, (u32)isr25, 0x08, 0x8E);
    idt_set_gate(26, (u32)isr26, 0x08, 0x8E);
    idt_set_gate(27, (u32)isr27, 0x08, 0x8E);
    idt_set_gate(28, (u32)isr28, 0x08, 0x8E);
    idt_set_gate(29, (u32)isr29, 0x08, 0x8E);
    idt_set_gate(30, (u32)isr30, 0x08, 0x8E);
    idt_set_gate(31, (u32)isr31, 0x08, 0x8E);

    irq_remap();

    // mapping IRQ handlers above the default CPU ones
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

    // TODO: other above 47, some use 128 for sys calls?
}

/// CPU Exception Message
///
/// http://wiki.osdev.org/Exceptions
///
///
c_str exception_messages[] =
{
    // 0x00 - 0x09
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",

    // CSO is on 386 and earlier only
    "Coprocessor Segment Overrun",

    // 0x0A-0x13
    "Bad TSS", // Invalid Task State Segment
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt [RESERVED]",
    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",

    // 19-??
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

// TODO: allow set on install
c_str irq_names[] =
{
    // 0-9
    "Timer",
    "Keyboard",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    // 10-16
    "",
    "",
    "Mouse",
    "",
    "",
    "",
    ""
};

static isr_handler irq_routines[IRQ_COUNT] = { 0, };
static u32 irq_counts[IRQ_COUNT] = { 0, };
//static u8 irq_names[IRQ_COUNT][20] = { {0, }, };

/// (Un)Install IRQ handler
#define __func__ "[FUNC MISSING]"
#define __line__ "[LINE MISSING]"
#define ASSERT(cond,msg) if(!(cond)) { kprintf("[ASSERT]: " __func__ " , " __line__ " : %s", msg); /*panic()*/; }
void irq_install_handler(u32 irq, isr_handler handler, c_str name)
{
    ASSERT(kstrlen(name) < 20, "name length must be < 20!");
    irq_routines[irq] = handler;
//    kmemcpyb((u8*)&irq_names[irq], (u8*)name, 20);
}
void irq_uninstall_handler(u32 irq)
{
    irq_routines[irq] = 0;
//    kmemsetb((u8*)&irq_names[irq], 0, 20);
}

/// Remap IRQs 0-7 since they are mapped to IDT entries 8-15 by default
/// IRQ 0 is mapped to IDT entry 8 is Double Fault
/// (only an issue in protected mode: )
void irq_remap()
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

void kernel_panic()
{
    kputs("\n\n ****  PANIC. System Halted! ******* \n\n");
    for (;;);
}

// TODO:
// - combine ISR w/IRQ (IRQs are mapped into IDT table, see above)
// - IDT supports 128 ISRs
// - fault_handler should handle some of the interrupts
// - at least show a panic screen

/// The hardware ISRs use this handler
/// cli() and sli() are called from asm
void fault_handler(isr_stack_state* r)
{
    kprintf("Unhandled Exception [ISR #%d]:", r->int_no);

    /// Should we handle this?
    if (r->int_no < 32)
    {
        kputs(exception_messages[r->int_no]);
    }

    kputs("\n");
    //kernel_panic();
}

const u8 END_OF_INTERRUPT = 0x20;
const u8 IRQ_BANK_MASTER = 0x20;
const u8 IRQ_BANK_SLAVE = 0xA0;

/// IRQs
/// Two 8259 chips: First bank: 0x20, Second: 0xA0.
/// IRQ handler - called from assembly
void irq_ack(u32 irq_no)
{
    // Need to send IRQ 8-15 to SLAVE as well
    if (irq_no >= 40) {
        outb(IRQ_BANK_SLAVE, END_OF_INTERRUPT);
    }
    outb(IRQ_BANK_MASTER, END_OF_INTERRUPT);
}
void irq_handler(isr_stack_state* r)
{
    // if we've installed a handler, call it
    u8 irq = r->int_no - 32;
    if (irq_routines[irq]) {
        isr_handler handler = irq_routines[irq];
        handler(r);
    }

    irq_ack(r->int_no);

    irq_counts[irq]++;
}

void print_irq_counts()
{
    for(int i=0; i < IRQ_COUNT; ++i) {
        kprintf("%d:\t%d\t'%s'", i, irq_counts[i], irq_names[i]);
    }
}

