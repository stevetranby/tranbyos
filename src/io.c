#include "include/system.h"

static inline u8 inb_dummy(u16 port)
{
    u8 ret;
    asm_volatile ( "inb %[port], %[ret]"
                  : [ret] "=a"(ret)
                  : [port] "Nd"(port) );
    return ret;
}

// TODO: maybe just move these into .asm? why bother just because
//       we want everything in C?

/* We will use this later on for reading from the I/O ports to get data
*  from devices such as the keyboard. We are using what is called
*  'inline assembly' in these routines to actually do the work */
u8 inb (u16 _port)
{
    u8 rv;
    asm_volatile ("inb %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

u16 inw(u16 _port)
{
    u16 rv;
    asm_volatile ("inw %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

u32 inl(u16 _port)
{
    u32 rv;
    asm_volatile ("inl %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

void outb(u16 _port, u8 _data)
{
    asm_volatile ("outb %1, %0" : : "dN" (_port), "a" (_data));
}

void outw(u16 _port, u16 _data)
{
    asm_volatile ("outw %1, %0" : : "dN" (_port), "a" (_data));
}

void outd(u16 _port, u32 _data)
{
    asm_volatile ("outl %1, %0" : : "dN" (_port), "a" (_data));
}

void print_port(u16 port)
{	
	printBinary_b(inb(port));	
}



static inline u32 farpeekl(u16 sel, void* off)
{
    u32 ret;
    asm_volatile ( "push %%fs\n\t"
                     "mov  %1, %%fs\n\t"
                     "mov  %%fs:(%2), %0\n\t"
                     "pop  %%fs"
                     : "=r"(ret) : "g"(sel), "r"(off) );
    return ret;
}

static inline void farpokeb(u16 sel, void* off, u8 v)
{
    asm_volatile ( "push %%fs\n\t"
                     "mov  %0, %%fs\n\t"
                     "movb %2, %%fs:(%1)\n\t"
                     "pop %%fs"
                     : : "g"(sel), "r"(off), "r"(v) );
    // TODO: Should "memory" be in the clobber list here?
}

// GCC has a <cpuid.h> header and intel has intrinsics header
bool cpuid_exists_by_eflags(void)
{
    // TODO: check if exists on x86 (always present on x86_64)
    // return 1;

    i32 result;
    asm_volatile (  "	pushfl\n"
                    "	pop	%%eax\n"
                    "	mov	%%eax,	%%ecx\n"
                    "	xor	$0x200000,	%%eax\n"
                    "	push	%%eax\n"
                    "	popfl\n"
                    "	pushfl\n"
                    "	pop	%%eax\n"
                    "	xor	%%ecx,	%%eax\n"
                    "	mov	%%eax,	%0\n"
                    "	push	%%ecx\n"
                    "	popfl\n"
                    : "=m"(result) : : "eax", "ecx", "memory");
    return (result != 0);
}
static inline void cpuid(int code, u32* a, u32* d)
{

    asm_volatile ( "cpuid" : "=a"(*a), "=d"(*d) : "0"(code) : "ebx", "ecx" );
}

/// CPU's time-stamp counter and store into EDX:EAX.
static inline u64 rdtsc()
{
    u64 ret;
    asm_volatile ( "rdtsc" : "=A"(ret) );
    return ret;
}

static inline bool are_interrupts_enabled()
{
    unsigned long flags;
    asm_volatile ( "pushf\n\t"
                  "pop %0"
                  : "=g"(flags) );
    return flags & (1 << 9);
}

static inline void lidt(void* base, u16 size)
{   // This function works in 32 and 64bit mode
    struct {
        u16 length;
        void*    base;
    } __attribute__((packed)) IDTR = { size, base };

    asm_volatile ( "lidt %0" : : "m"(IDTR) );  // let the compiler choose an addressing mode
}

static inline unsigned long read_cr0(void)
{
    unsigned long val;
    asm_volatile ( "mov %%cr0, %0" : "=r"(val) );
    return val;
}

// Invalidates the TLB (Translation Lookaside Buffer) for one specific virtual address. The next memory reference for the page will be forced to re-read PDE and PTE from main memory. Must be issued every time you update one of those tables. The m pointer points to a logical address, not a physical or virtual one: an offset for your ds segment.
inline void invlpg(void* m)
{
    /* Clobber memory to avoid optimizer re-ordering access before invlpg, which may cause nasty bugs. */
    asm_volatile ( "invlpg (%0)" : : "b"(m) : "memory" );
}

// Write a 64-bit value to a MSR. The A constraint stands for concatenation of registers EAX and EDX.
inline void wrmsr(u32 msr_id, u64 msr_value)
{
    asm_volatile ( "wrmsr" : : "c" (msr_id), "A" (msr_value) );
}

// Read a 64-bit value from a MSR. The A constraint stands for concatenation of registers EAX and EDX.
inline u64 rdmsr(u32 msr_id)
{
    u64 msr_value;
    asm_volatile ( "rdmsr" : "=A" (msr_value) : "c" (msr_id) );
    return msr_value;
}

//////////////////////////////////////////////////////////////////

// Serial Port Communication - mostly used for debug w/QEMU

#define PORT_COM1 0x3f8   /* COM1 */

void init_serial()
{
   outb(PORT_COM1 + 1, 0x00);    // Disable all interrupts
   outb(PORT_COM1 + 3, 0x80);    // Enable DLAB (set baud rate divisor)
   outb(PORT_COM1 + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
   outb(PORT_COM1 + 1, 0x00);    //                  (hi byte)
   outb(PORT_COM1 + 3, 0x03);    // 8 bits, no parity, one stop bit
   outb(PORT_COM1 + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
   outb(PORT_COM1 + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

int serial_received() {
   return inb(PORT_COM1 + 5) & 1;
}
 
char read_serial() {
   while (serial_received() == 0);
 
   return inb(PORT_COM1);
}

typedef int steveis_awesome_t;
steveis_awesome_t is_transmit_empty() {
   return inb(PORT_COM1 + 5) & 0x20;
}
 
void serial_write_b(u8 a) {
   while (is_transmit_empty() == 0)
   		; 
   outb(PORT_COM1,a);
}

void serial_writeln(c_str str) {
    while (*str != 0) {
        serial_write_b(*str);
        str++;
    }
    serial_write_b('\r');
    serial_write_b('\n');
}

void serial_write(c_str str) {
	while (*str != 0) {
		if(*str == '\n') {
			serial_write_b('\r');
			serial_write_b('\n');
		}
		serial_write_b(*str);
		str++;
	}
}