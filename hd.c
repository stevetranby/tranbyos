#include <system.h>
#include <hd.h>


void delay(void) {
  int t;
  for (t=0;t<1000;t++) __asm__("nop");
}

void delay400ns(void) {
	int i;
	for(i=0;i<4;++i)
		inb(HD_STATUS);	
}

// issue software reset
int reset_devices(void) {
	// write status
	outb(HD_STATUS, 0x02);
	delay400ns();
	outb(HD_DCR, HD_DCR_SRST);
	delay400ns();
	
	// If there is a device 0, wait for device 0 to set BSY=0
	int t;
	for(t=0; t<99999; ++t) {
		if((inb(HD_STATUS) & HD_STATUS_BSY) == 0) break;
	}
	if(t==99999) puts("Device 0 reset timeout\n");

	return 0;
}

/* on Primary bus: ctrl->base =0x1F0, ctrl->dev_ctl =0x3F6. REG_CYL_LO=4, REG_CYL_HI=5, REG_DEVSEL=6 */
void print_hd_device_types()
{		
	byte sc, sn, st, cl, ch;

	outb(HD_STATUS, 0x08);
	outb(HD_SELECT, 0xa0);
	delay400ns();
	outb(HD_NSECTOR,	0x55);
	outb(HD_SECTOR,	0xaa);
	outb(HD_NSECTOR,	0xaa);
	outb(HD_SECTOR,	0x55);
	outb(HD_NSECTOR,	0x55);
	outb(HD_SECTOR,	0xaa);
	sc = inb(HD_NSECTOR);
	sn = inb(HD_SECTOR);

	if((sc==0x55) && (sn==0xaa)) 
		puts("Primary Master Drive Unknown.\n");
	
	outb(HD_SELECT, 0xb0);
	delay400ns();
	outb(HD_NSECTOR,	0x55);
	outb(HD_SECTOR,	0xaa);
	outb(HD_NSECTOR,	0xaa);
	outb(HD_SECTOR,	0x55);
	outb(HD_NSECTOR,	0x55);
	outb(HD_SECTOR,	0xaa);
	sc = inb(HD_NSECTOR);
	sn = inb(HD_SECTOR);

	if((sc==0x55) && (sn==0xaa)) 
		puts("Primary Slave Drive Unknown.\n");

	// Soft Reset
	reset_devices();
	//outb(HD_DCR, HD_DCR_SRST);

	outb(HD_SELECT, 0xa0);
	delay400ns();
	
	sc = inb(HD_NSECTOR);
	sn = inb(HD_SECTOR);
	st = inb(HD_STATUS);
	if(st == 0x7f) {
		puts("No Drive\n");
	} else {
		if((sc==0x01) && (sn==0x01)) {
			cl = inb(HD_LOWCYL);
			ch = inb(HD_HIGHCYL);
			if((cl==0x14)&&(ch=0xeb))
				puts("Primary Master is ATAPI\n");
			else if ((cl==0x00) && (ch==0x00))
				puts("Primary Master is ATA\n");
			else {
				puts("Identify drive: cl="); printHex(cl);
				puts(" ch="); printHex(ch);
				putch('\n');
			}
		} else {
			puts("Identifying drive: sc="); printHex(sc);
			puts(" sn="); printHex(sn);
			putch('\n');
		}
	}

	outb(HD_SELECT, 0xb0);
	delay400ns();
	
	sc = inb(HD_NSECTOR);
	sn = inb(HD_SECTOR);
	st = inb(HD_STATUS);
	if(st == 0x7f) {
		puts("No Drive\n");
	} else {
		if((sc==0x01) && (sn==0x01)) {
			cl = inb(HD_LOWCYL);
			ch = inb(HD_HIGHCYL);
			if((cl==0x14)&&(ch=0xeb))
				puts("Primary Slave is ATAPI\n");
			else if ((cl==0x00) && (ch==0x00))
				puts("Primary Slave is ATA\n");
			else {
				puts("Identify drive: cl="); printHex(cl);
				puts(" ch="); printHex(ch);
				putch('\n');
			}
		} else {
			puts("Identifying drive: sc="); printHex(sc);
			puts(" sn="); printHex(sn);
			putch('\n');
		}
	}


	// Soft Reset
	reset_devices();

	// 2nd IDE Controller
	outb(HD2_STATUS, 0x08);
	outb(HD2_SELECT, 0xa0);
	delay400ns();
	outb(HD2_NSECTOR,	0x55);
	outb(HD2_SECTOR,	0xaa);
	outb(HD2_NSECTOR,	0xaa);
	outb(HD2_SECTOR,	0x55);
	outb(HD2_NSECTOR,	0x55);
	outb(HD2_SECTOR,	0xaa);
	sc = inb(HD2_NSECTOR);
	sn = inb(HD2_SECTOR);

	if((sc==0x55) && (sn==0xaa)) 
		puts("Secondary Master Drive Unknown.\n");
	
	outb(HD2_SELECT, 0xb0);
	delay400ns();
	outb(HD2_NSECTOR,	0x55);
	outb(HD2_SECTOR,	0xaa);
	outb(HD2_NSECTOR,	0xaa);
	outb(HD2_SECTOR,	0x55);
	outb(HD2_NSECTOR,	0x55);
	outb(HD2_SECTOR,	0xaa);
	sc = inb(HD2_NSECTOR);
	sn = inb(HD2_SECTOR);

	if((sc==0x55) && (sn==0xaa)) 
		puts("Secondary Slave Drive Unknown.\n");

	// Soft Reset
	reset_devices();
	//outb(HD2_DCR, HD_DCR_SRST);

	outb(HD_SELECT, 0xa0);
	delay400ns();
	
	sc = inb(HD2_NSECTOR);
	sn = inb(HD2_SECTOR);
	st = inb(HD2_STATUS);
	if(st == 0x7f) {
		puts("No Drive\n");
	} else {
		if((sc==0x01) && (sn==0x01)) {
			cl = inb(HD2_LOWCYL);
			ch = inb(HD2_HIGHCYL);
			if((cl==0x14)&&(ch=0xeb))
				puts("Secondary Master is ATAPI\n");
			else if ((cl==0x00) && (ch==0x00))
				puts("Secondary Master is ATA\n");
			else {
				puts("Identify drive: cl="); printHex(cl);
				puts(" ch="); printHex(ch);
				putch('\n');
			}
		} else {
			puts("Identifying drive: sc="); printHex(sc);
			puts(" sn="); printHex(sn);
			putch('\n');
		}
	}

	outb(HD_SELECT, 0xb0);
	delay400ns();
	
	sc = inb(HD2_NSECTOR);
	sn = inb(HD2_SECTOR);
	st = inb(HD2_STATUS);
	if(st == 0x7f) {
		puts("No Drive\n");
	} else {
		if((sc==0x01) && (sn==0x01)) {
			cl = inb(HD2_LOWCYL);
			ch = inb(HD2_HIGHCYL);
			if((cl==0x14)&&(ch=0xeb))
				puts("Secondary Slave is ATAPI\n");
			else if ((cl==0x00) && (ch==0x00))
				puts("Secondary Slave is ATA\n");
			else {
				puts("Identify drive: cl="); printHex(cl);
				puts(" ch="); printHex(ch);
				putch('\n');
			}
		} else {
			puts("Identifying drive: sc="); printHex(sc);
			puts(" sn="); printHex(sn);
			putch('\n');
		}
	}
/*
	uint16 i;
	byte data[512];
	for(i=0; i<512; ++i) {
		data[i] = inportb(HD_DATA);
	}
*/
}




/*
// Get information about the specified drive 
int hd_info(uint8 drive) {
	
}

int hd_read() {

}

int hd_write() {

}

int hd_command() {

}

// Send a soft-reset command 
int hd_softreset() {

}
*/

