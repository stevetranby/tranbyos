#include <system.h>
#include <hd.h>

void inline ata_delay400ns()
{
    inb(HD_ST);
    inb(HD_ST);
    inb(HD_ST);
    inb(HD_ST);
}

void ata_wait_busy() {
	byte status;
	int timer = 10000;
	while(1) {
		status = inb(HD_ST_ALT);
		if ( (status & HD_ST_BSY) ) {
			break;
		}
		if ( (status & HD_ST_ERR) ) {
			puts(" Error in ata_wait_drq! ");
		}
		if (--timer < 0) {
			trace(" Timeout in ata_wait_busy! ");
			break;
		}
	}
}

void ata_wait_drq() {
	byte status;
	int timer = 10000;
	while(1) {
		status = inb(HD_ST_ALT);
		if ( !(status & HD_ST_DRQ) ) {
			break;
		}
		if ( (status & HD_ST_ERR) ) {
			puts(" Error in ata_wait_drq! ");
		}
		if (--timer < 0) {
			trace(" Timeout in ata_wait_drq! ");
			break;
		}
	}
}

void ata_wait_ready() {
	byte status;
	int timer = 10000;
	while(1) {
		status = inb(HD_ST_ALT);
		if ( !(status & HD_ST_RDY) ) {
			break;
		}
		if ( (status & HD_ST_ERR) ) {
			puts(" Error in ata_wait_drq! ");
		}
		if (--timer < 0) {
			trace(" Timeout in ata_wait_ready! ");
			break;
		}
	}
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
    ata_wait_busy();
    return 0;
}



int ata_controller_present(int controller)
{
	int ret = 0;
	cli();
	outb(HD_SN, 0xa5);
	ata_delay400ns();
	byte temp = inb(HD_SN);
	if(temp == 0xa5) {
		ret = 1;
	}
	outb(HD_SN, 0x01);
	sti();
	return ret;
}

int ata_drive_present(int controller, int slave) {
	int ret = 0;
	cli();
	if(controller == IDE_PRIMARY) {
		outb(HD_DH, 0xa0 | slave << 4);
		ata_delay400ns();
		byte temp = inb(HD_ST);
		if(temp & HD_ST_BSY) {
			ret = 1;
		}
	} else {
		outb(HD1_DH, 0xa0 | slave << 4);
		ata_delay400ns();
		byte temp = inb(HD1_ST);
		if(temp & HD_ST_BSY) {
			ret = 1;
		}
	}
	sti();
	return ret;
}

// controller - 0=primary, 1=secondary
// slave - 0 or 1 depending on if slave drive
int ata_pio_write_w(int controller, int slave, int sn, int sc, word *data)
{
    int i;

    outb(HD_DH, 0xE0 | slave << 4);
    outb(HD_SC, sc);
    outb(HD_SN, 0x01);
    outb(HD_CL, 0x00);
    outb(HD_CH, 0x00);
    outb(HD_CMD, HD_CMD_WRITE);

    ata_wait_drq();

    trace("\nWriting data: ");
    for(i=0; i < sc*256; ++i)
    {
        outw(HD_DATA, data[i]);
        if((inb(HD_ST_ALT)&HD_ST_ERR)) {
        	trace("\nError Occured: "); print_port(HD_ERR);
        }
    }
    trace(" Finished Writing");

    return 1;
}

// controller - 0=primary, 1=secondary
// slave - 0 or 1 depending on if slave drive
int ata_pio_read_w(int controller, int slave, int sn, int sc, word *data)
{
    int i=0;
    // get the sector count from data size

    outb(HD_DH, 0xE0 | slave << 4);
    outb(HD_SC, sc);
    outb(HD_SN, 0x01);
    outb(HD_CL, 0x00);
    outb(HD_CH, 0x00);
    outb(HD_CMD, HD_CMD_READ);

    ata_wait_drq();

    trace("\nReading data: ");
    for(i=0; i < sc*256; ++i)
    {
        data[i] = inw(HD_DATA);
        if((inb(HD_ST_ALT)&HD_ST_ERR)) {
        	trace("\nError Occured: "); print_port(HD_ERR);
        }
    }
    trace(" Finished Reading");

    return 1;
}

// Calculate CHS to BYTES
uint32 chs2bytes(word c, word h, word s) {
  uint32 sectorbytes = SECTOR_BYTES;
  uint32 bytes = sectorbytes*c*h*s;
//  uint32 kilobytes = bytes/1024;
//  uint32 megabytes = bytes/1048576;
//  uint32 gigabytes = bytes/1073741824;
  return bytes;
}
