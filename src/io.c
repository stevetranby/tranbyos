#include "include/system.h"

#define global static
#define internal static inline
#define asm_volatile __asm__ __volatile__

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
    __asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

u16 inw(u16 _port)
{
    u16 rv;
    __asm__ volatile ("inw %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

u32 ind(u16 _port)
{
    u32 rv;
    __asm__ volatile ("inl %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

/* We will use this to write to I/O ports to send bytes to devices. This
*  will be used in the next tutorial for changing the textmode cursor
*  position. Again, we use some inline assembly for the stuff that simply
*  cannot be done in C */
void outb (u16 _port, u8 _data)
{
__asm__ __volatile__ ("outb %1, %0" : : "dN" (_port), "a" (_data));
}

void outw (u16 _port, u16 _data)
{
__asm__ __volatile__ ("outw %1, %0" : : "dN" (_port), "a" (_data));
}

void outd (u16 _port, u32 _data)
{
__asm__ __volatile__ ("outl %1, %0" : : "dN" (_port), "a" (_data));
}

void print_port(u16 port)
{	
	printBinary_b(inb(port));	
}


//////////////////////////////////////////////////////

// Serial Port Communication - mostly used for debug w/QEMU

#define PORT 0x3f8   /* COM1 */

void init_serial() {
   outb(PORT + 1, 0x00);    // Disable all interrupts
   outb(PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
   outb(PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
   outb(PORT + 1, 0x00);    //                  (hi byte)
   outb(PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
   outb(PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
   outb(PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

int serial_received() {
   return inb(PORT + 5) & 1;
}
 
char read_serial() {
   while (serial_received() == 0);
 
   return inb(PORT);
}

typedef int steveis_awesome_t;
steveis_awesome_t is_transmit_empty() {
   return inb(PORT + 5) & 0x20;
}
 
void serial_write_b(u8 a) {
   while (is_transmit_empty() == 0)
   		; 
   outb(PORT,a);
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