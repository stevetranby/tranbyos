#include <system.h>

/* We will use this later on for reading from the I/O ports to get data
*  from devices such as the keyboard. We are using what is called
*  'inline assembly' in these routines to actually do the work */
uint8 inb (uint16 _port)
{
    byte rv;
__asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

word inw(word _port)
{
    word rv;
__asm__ volatile ("inw %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

dword ind(word _port)
{
    dword rv;
__asm__ volatile ("inl %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

/* We will use this to write to I/O ports to send bytes to devices. This
*  will be used in the next tutorial for changing the textmode cursor
*  position. Again, we use some inline assembly for the stuff that simply
*  cannot be done in C */
void outb (word _port, byte _data)
{
__asm__ __volatile__ ("outb %1, %0" : : "dN" (_port), "a" (_data));
}

void outw (word _port, word _data)
{
__asm__ __volatile__ ("outw %1, %0" : : "dN" (_port), "a" (_data));
}

void outd (word _port, dword _data)
{
__asm__ __volatile__ ("outl %1, %0" : : "dN" (_port), "a" (_data));
}

void print_port(uint16 port)
{	
	printBin_b(inb(port));	
}
