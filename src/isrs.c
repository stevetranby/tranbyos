#include <system.h>

// Interrupts & CPU Exception Handling
// - http://wiki.osdev.org/Interrupts
// - http://wiki.osdev.org/Exceptions
// - https://en.wikipedia.org/wiki/Interrupt_descriptor_table

void disable_irq(int irq);
void disable_irq_nosync(int irq);
void enable_irq(int irq);

// Disable PIC if using APIC and IOAPIC
//mov al, 0xff
//out 0xa1, al
//out 0x21, al

////////////////////////////////////////////////////////////////////////////////////
/// IDT

#define MAX_INTERRUPTS 256

// Store 256 interrupt entries, 32 ISR, 16 IRQ, rest "unhandled interrupt" exception
idt_entry idt[MAX_INTERRUPTS];
idt_ptr   idtp;

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
    isrs_install();

    // Points the processor's internal register to the new IDT
    idt_load();
}

//////////////////////////////////////////


// TODO: linux v1 source

/*
 
#define get_seg_byte(seg,addr) ({ \
register char __res; \
__asm__("push %%fs;mov %%ax,%%fs;movb %%fs:%2,%%al;pop %%fs" \
:"=a" (__res):"0" (seg),"m" (*(addr))); \
__res;})

#define get_seg_long(seg,addr) ({ \
register unsigned long __res; \
__asm__("push %%fs;mov %%ax,%%fs;movl %%fs:%2,%%eax;pop %%fs" \
:"=a" (__res):"0" (seg),"m" (*(addr))); \
__res;})

#define _fs() ({ \
register unsigned short __res; \
__asm__("mov %%fs,%%ax":"=a" (__res):); \
__res;})

#define str(n) \
__asm__("str %%ax\n\t" \
"subl %2,%%eax\n\t" \
"shrl $4,%%eax" \
:"=a" (n) \
:"a" (0),"i" (FIRST_TSS_ENTRY<<3))

 */


//static void panic_exception(char * str,long esp_ptr,long nr)
//{
//    long * esp = (long *) esp_ptr;
//    int i;
//
//    kprintk("%s: %04x\n\r",str,nr&0xffff);
//    kprintk("EIP:\t%04x:%p\nEFLAGS:\t%p\nESP:\t%04x:%p\n",
//           esp[1],esp[0],esp[2],esp[4],esp[3]);
//    kprintk("fs: %04x\n",_fs());
//    kprintk("base: %p, limit: %p\n",get_base(current->ldt[1]),get_limit(0x17));
//    if (esp[4] == 0x17) {
//        kprintk("Stack: ");
//        for (i=0;i<4;i++)
//            kprintk("%p ",get_seg_long(0x17,i+(long *)esp[3]));
//        kprintk("\n");
//    }
//    str(i);
//    kprintk("Pid: %d, process nr: %d\n\r",current->pid,0xffff & i);
//    for(i=0;i<10;i++)
//        kprintk("%02x ",0xff & get_seg_byte(esp[1],(i+(char *)esp[0])));
//    kprintk("\n\r");
//
//
//    do_exit(11);
//}


//////////////////////////////////////////////////////////////////
// ISRS

/// Setup the IDT with to map interrupts to correct handler
void isrs_install()
{
    // TODO: use an "unused" stub/handler for those not used??
    // TODO: code generator or move this into NASM where we have repeat macro

#define IDT_GATE_INTERRUPT  0x8E // (dpl protection level max - ring = 0, Gate Type = 0xE)
#define IDT_GATE_TRAP       0x8F // (dpl protection level max - ring = 0, Gate Type = 0xF)
#define IDT_GATE_SYSTEM     0xEF // (dpl protection level max - ring = 3, Gate Type = 0xF)

    idt_set_gate(0, (u32)isr0, 0x08, IDT_GATE_TRAP);
    idt_set_gate(1, (u32)isr1, 0x08, IDT_GATE_TRAP);
    idt_set_gate(2, (u32)isr2, 0x08, IDT_GATE_TRAP);
    idt_set_gate(3, (u32)isr3, 0x08, IDT_GATE_SYSTEM);
    idt_set_gate(4, (u32)isr4, 0x08, IDT_GATE_SYSTEM);
    idt_set_gate(5, (u32)isr5, 0x08, IDT_GATE_SYSTEM);
    idt_set_gate(6, (u32)isr6, 0x08, IDT_GATE_TRAP);
    idt_set_gate(7, (u32)isr7, 0x08, IDT_GATE_TRAP);
    idt_set_gate(8, (u32)isr8, 0x08, IDT_GATE_TRAP);
    idt_set_gate(9, (u32)isr9, 0x08, IDT_GATE_TRAP);
    idt_set_gate(10, (u32)isr10, 0x08, IDT_GATE_TRAP);
    idt_set_gate(11, (u32)isr11, 0x08, IDT_GATE_TRAP);
    idt_set_gate(12, (u32)isr12, 0x08, IDT_GATE_TRAP);
    idt_set_gate(13, (u32)isr13, 0x08, IDT_GATE_TRAP);
    idt_set_gate(14, (u32)isr14, 0x08, IDT_GATE_TRAP);
    idt_set_gate(15, (u32)isr15, 0x08, IDT_GATE_TRAP);
    idt_set_gate(16, (u32)isr16, 0x08, IDT_GATE_TRAP);
    idt_set_gate(17, (u32)isr17, 0x08, IDT_GATE_TRAP);
    idt_set_gate(18, (u32)isr18, 0x08, IDT_GATE_TRAP);
    idt_set_gate(19, (u32)isr19, 0x08, IDT_GATE_TRAP);
    idt_set_gate(20, (u32)isr20, 0x08, IDT_GATE_TRAP);
    idt_set_gate(21, (u32)isr21, 0x08, IDT_GATE_TRAP);
    idt_set_gate(22, (u32)isr22, 0x08, IDT_GATE_TRAP);
    idt_set_gate(23, (u32)isr23, 0x08, IDT_GATE_TRAP);
    idt_set_gate(24, (u32)isr24, 0x08, IDT_GATE_TRAP);
    idt_set_gate(25, (u32)isr25, 0x08, IDT_GATE_TRAP);
    idt_set_gate(26, (u32)isr26, 0x08, IDT_GATE_TRAP);
    idt_set_gate(27, (u32)isr27, 0x08, IDT_GATE_TRAP);
    idt_set_gate(28, (u32)isr28, 0x08, IDT_GATE_TRAP);
    idt_set_gate(29, (u32)isr29, 0x08, IDT_GATE_TRAP);
    idt_set_gate(30, (u32)isr30, 0x08, IDT_GATE_TRAP);
    idt_set_gate(31, (u32)isr31, 0x08, IDT_GATE_TRAP);

    // Remap IRQs from 0 and 8 to => 32 and 40
    irq_remap(0x20, 0x28);

    // mapping IRQ handlers above the default CPU ones
    idt_set_gate(32, (u32)irq0, 0x08, IDT_GATE_INTERRUPT);
    idt_set_gate(33, (u32)irq1, 0x08, IDT_GATE_INTERRUPT);
    idt_set_gate(34, (u32)irq2, 0x08, IDT_GATE_INTERRUPT);
    idt_set_gate(35, (u32)irq3, 0x08, IDT_GATE_INTERRUPT);
    idt_set_gate(36, (u32)irq4, 0x08, IDT_GATE_INTERRUPT);
    idt_set_gate(37, (u32)irq5, 0x08, IDT_GATE_INTERRUPT);
    idt_set_gate(38, (u32)irq6, 0x08, IDT_GATE_INTERRUPT);
    idt_set_gate(39, (u32)irq7, 0x08, IDT_GATE_INTERRUPT);
    idt_set_gate(40, (u32)irq8, 0x08, IDT_GATE_INTERRUPT);
    idt_set_gate(41, (u32)irq9, 0x08, IDT_GATE_INTERRUPT);
    idt_set_gate(42, (u32)irq10, 0x08, IDT_GATE_INTERRUPT);
    idt_set_gate(43, (u32)irq11, 0x08, IDT_GATE_INTERRUPT);
    idt_set_gate(44, (u32)irq12, 0x08, IDT_GATE_INTERRUPT);
    idt_set_gate(45, (u32)irq13, 0x08, IDT_GATE_INTERRUPT);
    idt_set_gate(46, (u32)irq14, 0x08, IDT_GATE_INTERRUPT);
    idt_set_gate(47, (u32)irq15, 0x08, IDT_GATE_INTERRUPT);

    // TODO: other above 47, some use 128 for sys calls?
    //idt_set_gate(47, (u32)irq127, 0x08, );
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

static u32 isr_counts[ISR_COUNT] = { 0, };
static isr_handler isr_stubs[ISR_COUNT] = { 0, };

static isr_handler irq_routines[IRQ_COUNT] = { 0, };
static u32 irq_counts[IRQ_COUNT] = { 0, };
static u8 irq_names[IRQ_COUNT][20] = { {0, }, };

/// (Un)Install IRQ handler

void irq_install_handler(u32 irq, isr_handler handler, c_str name)
{
    ASSERT(kstrlen(name) < 20, "name length must be < 20!\n");
    irq_routines[irq] = handler;
    kmemcpyb((u8*)irq_names[irq], (u8*)name, 20);
    kwritef(serial_write_b, "installing irq: %d, %x, '%s'\n", irq, (u32)handler, name);
}
void irq_uninstall_handler(u32 irq)
{
    irq_routines[irq] = 0;
    kmemsetb((u8*)&irq_names[irq], 0, 20);
}

// May be fragile? basically does an opcode for jmp twice
#define PIC_WAIT() do { asm volatile("jmp 1f\n\t1:\n\tjmp 2f\n\t2:"); } while (0)


#define PIC1_CMD            0x20
#define PIC1_DATA           0x21
#define PIC2_CMD            0xA0
#define PIC2_DATA           0xA1

#define PIC_CMD_EOI         0x20   // end of interrupt

#define ICW1_ICW4           0x01   // ICW4 (not) needed
#define ICW1_SINGLE         0x02   // Single (cascade) mode
#define ICW1_INTERVAL4      0x04   // Call address interval 4 (8)
#define ICW1_LEVEL		  	0x08   // Level triggered (edge) mode
#define ICW1_INIT           0x10   // Initialization - required!
#define ICW4_8086           0x01   // 8086/88 (MCS-80/85) mode
#define ICW4_AUTO           0x02   // Auto (normal) EOI
#define ICW4_BUF_SLAVE      0x08   // Buffered mode/slave
#define ICW4_BUF_MASTER     0x0C   // Buffered mode/master
#define ICW4_SFNM           0x10   // Special fully nested (not)

/// Remap IRQs 0-7 since they are mapped to IDT entries 8-15 by default
/// IRQ 0 is mapped to IDT entry 8 is Double Fault (only an issue in protected mode: )
void irq_remap(int irqPrimary, int irqSecondary)
{
    // NOTE: other implementations don't bother saving/restoring the masks

    // save masks
    u8 a1 = inb(PIC1_DATA);
    u8 a2 = inb(PIC2_DATA);

    // TODO: name consts
    outb(PIC1_CMD, ICW1_INIT|ICW1_ICW4); PIC_WAIT();
    outb(PIC2_CMD, ICW1_INIT|ICW1_ICW4); PIC_WAIT();

    // remap
    outb(PIC1_DATA, irqPrimary); PIC_WAIT();
    outb(PIC2_DATA, irqSecondary); PIC_WAIT();

    // cascade identity with slave
    outb(PIC1_DATA, ICW1_INTERVAL4); PIC_WAIT();
    outb(PIC2_DATA, ICW1_SINGLE); PIC_WAIT();

    // Request 8086 mode on each PIC
    outb(PIC1_DATA, ICW4_8086); PIC_WAIT();
    outb(PIC2_DATA, ICW4_8086); PIC_WAIT();


//    outb(PIC1_DATA, 0x00);
//    outb(PIC2_DATA, 0x00);
//
//    // reset PIC's
//    outb(PIC1_DATA, 0x20);
//    outb(PIC2_DATA, 0x20);


    // restore saved masks.
    outb(PIC1_DATA, a1);
    outb(PIC2_DATA, a2);
}


void kernel_panic()
{
    kputs("\n\n ****  PANIC. System Halted! ******* \n\n");



    for (;;);
}

void do_int3(u32* esp, u32 error_code, u32 fs, u32 es, u32 ds,
             u32 ebp, u32 esi, u32 edi, u32 edx, u32 ecx, u32 ebx,u32 eax)
{
    int tr;
    __asm__("str %%ax":"=a" (tr):"0" (0));
    kprintf("\n\neax\t\tebx\t\tecx\t\tedx\n\r%x\t%x\t%x\t%x\n\r",
            eax,ebx,ecx,edx);
    kprintf("esi\t\tedi\t\tebp\t\tesp\n\r%x\t%x\t%x\t%x\n\r",
            esi,edi,ebp,(long) esp);
    kprintf("\n\rds\tes\tfs\ttr\n\r%x\t%x\t%x\t%x\n\r",
            ds,es,fs,tr);
    kprintf("\nEIP: %x   CS: %x  EFLAGS: %x\n\r",esp[0],esp[1],esp[2]);
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
    int i = r->int_no;
    /// Should we handle this?
    if (i < 32)
    {
        isr_handler handler = isr_stubs[i];
        if(handler) {
            // handle
            handler(r);
        } else if(i == 3) {
            do_int3((u32*)r->esp, r->err_code, r->fs, r->es, r->ds, r->ebp, r->esi, r->edi, r->edx, r->ecx, r->ebx, r->eax);
        } else {
            kprintf("Unhandled Exception [ISR #%d]: %s", i, exception_messages[i]);

            do_int3((u32*)r->esp, r->err_code, r->fs, r->es, r->ds, r->ebp, r->esi, r->edi, r->edx, r->ecx, r->ebx, r->eax);

            kernel_panic();
        }
    } else {
        kputs("Not Handling\n");
    }

    ++isr_counts[i];
}

//void irq_set_mask(unsigned char irqLine)
//{
//    uint16_t port;
//    uint8_t value;
//
//    if(IRQline < 8) {
//        port = PIC1_DATA;
//    } else {
//        port = PIC2_DATA;
//        IRQline -= 8;
//    }
//    value = inb(port) | (1 << IRQline);
//    outb(port, value);
//}
//
//void irq_clear_mask(unsigned char IRQline)
//{
//    uint16_t port;
//    uint8_t value;
//
//    if(IRQline < 8) {
//        port = PIC1_DATA;
//    } else {
//        port = PIC2_DATA;
//        IRQline -= 8;
//    }
//    value = inb(port) & ~(1 << IRQline);
//    outb(port, value);
//}
//




/// Two 8259 chips: First bank: 0x20, Second: 0xA0.
/// IRQ handler - called from assembly
// NOTE: irq_no is 0-15
void irq_ack(u32 irq_no)
{
    // Need to send IRQ 8-15 to SLAVE as well
    if (irq_no >= 8) {
        outb(PIC2_CMD, PIC_CMD_EOI);
    }
    outb(PIC1_CMD, PIC_CMD_EOI);
}

void irq_handler(isr_stack_state* r)
{
    // if we've installed a handler, call it
    u8 irq = r->int_no - 32;
    if (irq_routines[irq]) {
        isr_handler handler = irq_routines[irq];
        handler(r);
    }

    irq_ack(irq);

    irq_counts[irq]++;
}

void print_irq_counts()
{
    for(int i=0; i < IRQ_COUNT; ++i) {
        kprintf("%d:\t%d\t'%s'\n", i, irq_counts[i], irq_names[i]);
    }
}


///////
// Spurious IRQs

// http://wiki.osdev.org/PIC

/*
 When an IRQ occurs, the PIC chip tells the CPU (via. the PIC's INTR line) that there's an interrupt,
 and the CPU acknowledges this and waits for the PIC to send the interrupt vector. This creates a race
 condition: if the IRQ disappears after the PIC has told the CPU there's an interrupt but before the PIC
 has sent the interrupt vector to the CPU, then the CPU will be waiting for the PIC to tell it which
 interrupt vector but the PIC won't have a valid interrupt vector to tell the CPU.
 To get around this, the PIC tells the CPU a fake interrupt number. This is a spurious IRQ. The fake
 interrupt number is the lowest priority interrupt number for the corresponding PIC chip (IRQ 7 for the
 master PIC, and IRQ 15 for the slave PIC).
 There are several reasons for the interrupt to disappear. In my experience the most common reason is
 software sending an EOI at the wrong time. Other reasons include noise on IRQ lines (or the INTR line).

 Handling Spurious IRQs

 For a spurious IRQ, there is no real IRQ and the PIC chip's ISR (In Service Register) flag for the
 corresponding IRQ will not be set. This means that the interrupt handler must not send an EOI back to
 the PIC to reset the ISR flag.

 The correct way to handle an IRQ 7 is to first check the master PIC chip's ISR to see if the IRQ is a
 spurious IRQ or a real IRQ. If it is a real IRQ then it is treated the same as any other real IRQ.
 If it is a spurious IRQ then you ignore it (and do not send the EOI).

 The correct way to handle an IRQ 15 is similar, but a little trickier due to the interaction between
 the slave PIC and the master PIC. First check the slave PIC chip's ISR to see if the IRQ is a spurious
 IRQ or a real IRQ. If it is a real IRQ then it is treated the same as any other real IRQ. If it's a
 spurious IRQ then don't send the EOI to the slave PIC; however you will still need to send the EOI to
 the master PIC because the master PIC itself won't know that it was a spurious IRQ from the slave.
 Also note that some operating systems (e.g. Linux) keep track of the number of spurious IRQs that have
 occurred (e.g. by incrementing a counter when a spurious IRQ occurs). This can be useful for detecting
 problems in software (e.g. sending EOIs at the wrong time) and detecting problems in hardware
 (e.g. line noise).
 
 */
//#define PIC1_CMD                    0x20
//#define PIC1_DATA                   0x21
//#define PIC2_CMD                    0xA0
//#define PIC2_DATA                   0xA1
#define PIC_READ_IRR                0x0a    /* OCW3 irq ready next CMD read */
#define PIC_READ_ISR                0x0b    /* OCW3 irq service next CMD read */

/* Helper func */
static uint16_t __pic_get_irq_reg(int ocw3)
{
    /* OCW3 to PIC CMD to get the register values.  PIC2 is chained, and
     * represents IRQs 8-15.  PIC1 is IRQs 0-7, with 2 being the chain */
    outb(PIC1_CMD, ocw3);
    outb(PIC2_CMD, ocw3);
    return (inb(PIC2_CMD) << 8) | inb(PIC1_CMD);
}

/* Returns the combined value of the cascaded PICs irq request register */
uint16_t pic_get_irr(void)
{
    return __pic_get_irq_reg(PIC_READ_IRR);
}

/* Returns the combined value of the cascaded PICs in-service register */
uint16_t pic_get_isr(void)
{
    return __pic_get_irq_reg(PIC_READ_ISR);
}