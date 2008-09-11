#include <system.h>
#include <hd.h>

void inline ata_delay400ns()
{
    inb(HD_ST);
    inb(HD_ST);
    inb(HD_ST);
    inb(HD_ST);
}

// issue software reset
int ata_soft_reset(void)
{
    // Send Soft Reset
    outb(HD_DCR, HD_DCR_SRST);
    // Clear the register for normal operation
    outb(HD_DCR, 0x00);
    // wait 400ns
    ata_delay400ns();
    // wait while busy and not ready
    while((inb(HD_ST_ALT) & 0xc0) != 0x40)
    {}

    return 0;
}

int ata_controller_present(int controller)
{
	outb(HD_SN, 0xa5);
	ata_delay400ns();
	byte temp = inb(HD_SN);
	if(temp == 0xa5) {
		return 1;
	}
	return 0;
}

int ata_drive_present(int controller, int slave) {	
	outb(HD_DH, 0xa0 | slave << 4);
	ata_delay400ns();
	byte temp = inb(HD_ST);
	if(temp & HD_ST_BSY) {		
		return 1;
	}
	return 0;
}

// controller - 0=primary, 1=secondary
// slave - 0 or 1 depending on if slave drive
int ata_pio_write_w(int controller, int slave, int sn, word *data, int n)
{
    int i;
    // get the sector count from data size
    int sc = ((n + (SECTOR_WORDS/2)) / SECTOR_WORDS) + 1;

    outb(HD_DH, 0xA0 | slave << 4);
    outb(HD_SC, sc);
    outb(HD_SN, 0x01);
    outb(HD_CL, 0x00);
    outb(HD_CH, 0x00);
    outb(HD_CMD, HD_CMD_WRITE);

    while( !(inb(HD_ST) & 0x08) )
    {
        print_port(HD_ST);
    }


    trace("\nWriting data: ");
    for(i=0; i < n; ++i)
    {
        outw(HD_DATA, data[i]);
        print_port(HD_ST);
    }
    trace(" Finished Writing");

    return 1;
}

// controller - 0=primary, 1=secondary
// slave - 0 or 1 depending on if slave drive
int ata_pio_read_w(int controller, int slave, int sn, word *data, int n)
{
    int i=0;
    // get the sector count from data size
    int sc = ((n + (SECTOR_WORDS/2)) / SECTOR_WORDS) + 1;

    outb(HD_DH, 0xA0 | slave << 4);
    outb(HD_SC, sc);
    outb(HD_SN, 0x01);
    outb(HD_CL, 0x00);
    outb(HD_CH, 0x00);
    outb(HD_CMD, HD_CMD_READ);

    while( !(inb(0x1F7) & 0x08) )
    {
        print_port(HD_ST);
    }

    trace("\nReading data: ");
    for(i=0; i < n; ++i)
    {
        outw(HD_DATA, data[i]);
        print_port(HD_ST);
    }
    trace(" Finished Reading");

    return 1;
}
