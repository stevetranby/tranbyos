#include <system.h>
#include <hd.h>


void delay(void) {
  int t;
  for (t=0;t<1000;t++) __asm__("nop");
}

void delay400ns(void) {
	int i;
	for(i=0;i<4;++i)
		inb(HD_ST);	
}

// issue software reset
int reset_devices(void) {
	// write status
	outb(HD_ST, 0x02);
	delay400ns();
	outb(HD_DCR, HD_DCR_SRST);
	delay400ns();
	
	// If there is a device 0, wait for device 0 to set BSY=0
	int t;
	for(t=0; t<99999; ++t) {
		if((inb(HD_ST) & HD_ST_BSY) == 0) break;
	}
	if(t==99999) puts("Device 0 reset timeout\n");

	return 0;
}

void hd_print_status(void) {
	uint8 status = inportb(0x1F7);
	
	uint8 status_execmd = status & 0x80; // 1000 0000
	uint8 status_drvrdy = status & 0x40; // 0100 0000
	uint8 status_wrtflt = status & 0x20; // 0010 0000
	uint8 status_skcmpl = status & 0x10; // 0001 0000
	uint8 status_sbrqsv = status & 0x08; // 0000 1000
	uint8 status_ddrcor = status & 0x04; // 0000 0100
	uint8 status_idxrev = status & 0x02; // 0000 0010
	uint8 status_cmderr = status & 0x01; // 0000 0001

	
	printInt(status_execmd >> 7);
	printInt(status_drvrdy >> 6);
	printInt(status_wrtflt >> 5);
	printInt(status_skcmpl >> 4);
	printInt(status_sbrqsv >> 3);
	printInt(status_ddrcor >> 2);
	printInt(status_idxrev >> 1);
	printInt(status_cmderr);

	int i;
	for(i=0;i<8;++i) putch('\b');
}

void hd_write_b(uint32 sn, byte *data, int n, uint8 slave) {
	int sc = 1 + n/SECTOR_BYTES;
	int cl = 0;
	int ch = 0;
	
	// prime the controller
	outb(HD_DH, 0xA0 | (slave << 4)); // select device
	outb(HD_SC, sc); // sector count port, read 1 sector
	outb(HD_SN, sn); // sector # port, read sector 1
	outb(HD_CL, cl); // cyl low port, cyl 0
	outb(HD_CH, ch); // cyl high port, rest of cyl 0
	outb(HD_CMD, HD_CMD_WRITE); // cmd port, read with retry

	// Wait Busy
	while( ((inb(HD_ST) & (HD_ST_BSY | HD_ST_DRQ)) != HD_ST_DRQ) || (inb(HD_ST) & HD_ST_ERR) ) { hd_print_status(); }

	if(inb(HD_ST) & HD_ST_ERR) return;
	int i;
	for(i=0; i<(sc*SECTOR_BYTES); ++i) {
		outb(0x1F0, data[i]);
	}
}

void hd_read_b(uint32 sn, uint32 sc, uint8 slave) {
	// read the bytes of this sector
	byte data[sc*SECTOR_BYTES];	
	int cl = 0;
	int ch = 0;
	
	// prime the controller
	outb(HD_DH, 0xA0 | (slave << 4)); // select device
	outb(HD_SC, sc); // sector count port, read 1 sector
	outb(HD_SN, sn); // sector # port, read sector 1
	outb(HD_CL, cl); // cyl low port, cyl 0
	outb(HD_CH, ch); // cyl high port, rest of cyl 0
	outb(HD_CMD, HD_CMD_READ); // cmd port, read with retry

	// Wait Busy
	while( (inportb(HD_ST) & HD_ST_BSY) || (inb(HD_ST) & HD_ST_ERR) ) { hd_print_status(); }

	// on error return
	if(inb(HD_ST) & HD_ST_ERR) return;

	int i;
	for(i=0; i<(sc*SECTOR_BYTES); ++i) {
		data[i] = inb(0x1F0);
		printHex(data[i]); putch(',');
	}
}

/* on Primary bus: ctrl->base =0x1F0, ctrl->dev_ctl =0x3F6. REG_CYL_LO=4, REG_CYL_HI=5, REG_DEVSEL=6 */
void print_hd_device_types()
{		
	byte sc, sn, st, cl, ch;

	outb(HD_ST, 0x08);
	outb(HD_DH, 0xa0);
	delay400ns();
	outb(HD_SC,	0x55);
	outb(HD_SN,	0xaa);
	outb(HD_SC,	0xaa);
	outb(HD_SN,	0x55);
	outb(HD_SC,	0x55);
	outb(HD_SN,	0xaa);
	sc = inb(HD_SC);
	sn = inb(HD_SN);

	if((sc==0x55) && (sn==0xaa)) 
		puts("Primary Master Drive Unknown.\n");
	
	outb(HD_DH, 0xb0);
	delay400ns();
	outb(HD_SC,	0x55);
	outb(HD_SN,	0xaa);
	outb(HD_SC,	0xaa);
	outb(HD_SN,	0x55);
	outb(HD_SC,	0x55);
	outb(HD_SN,	0xaa);
	sc = inb(HD_SC);
	sn = inb(HD_SN);

	if((sc==0x55) && (sn==0xaa)) 
		puts("Primary Slave Drive Unknown.\n");

	// Soft Reset
	reset_devices();
	//outb(HD_DCR, HD_DCR_SRST);

	outb(HD_DH, 0xa0);
	delay400ns();
	
	sc = inb(HD_SC);
	sn = inb(HD_SN);
	st = inb(HD_ST);
	if(st == 0x7f) {
		puts("No Drive\n");
	} else {
		if((sc==0x01) && (sn==0x01)) {
			cl = inb(HD_CL);
			ch = inb(HD_CH);
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

	outb(HD_DH, 0xb0);
	delay400ns();
	
	sc = inb(HD_SC);
	sn = inb(HD_SN);
	st = inb(HD_ST);
	if(st == 0x7f) {
		puts("No Drive\n");
	} else {
		if((sc==0x01) && (sn==0x01)) {
			cl = inb(HD_CL);
			ch = inb(HD_CH);
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
	outb(HD2_ST, 0x08);
	outb(HD2_DH, 0xa0);
	delay400ns();
	outb(HD2_SC,	0x55);
	outb(HD2_SN,	0xaa);
	outb(HD2_SC,	0xaa);
	outb(HD2_SN,	0x55);
	outb(HD2_SC,	0x55);
	outb(HD2_SN,	0xaa);
	sc = inb(HD2_SC);
	sn = inb(HD2_SN);

	if((sc==0x55) && (sn==0xaa)) 
		puts("Secondary Master Drive Unknown.\n");
	
	outb(HD2_DH, 0xb0);
	delay400ns();
	outb(HD2_SC,	0x55);
	outb(HD2_SN,	0xaa);
	outb(HD2_SC,	0xaa);
	outb(HD2_SN,	0x55);
	outb(HD2_SC,	0x55);
	outb(HD2_SN,	0xaa);
	sc = inb(HD2_SC);
	sn = inb(HD2_SN);

	if((sc==0x55) && (sn==0xaa)) 
		puts("Secondary Slave Drive Unknown.\n");

	// Soft Reset
	reset_devices();
	//outb(HD2_DCR, HD_DCR_SRST);

	outb(HD_DH, 0xa0);
	delay400ns();
	
	sc = inb(HD2_SC);
	sn = inb(HD2_SN);
	st = inb(HD2_ST);
	if(st == 0x7f) {
		puts("No Drive\n");
	} else {
		if((sc==0x01) && (sn==0x01)) {
			cl = inb(HD2_CL);
			ch = inb(HD2_CH);
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

	outb(HD_DH, 0xb0);
	delay400ns();
	
	sc = inb(HD2_SC);
	sn = inb(HD2_SN);
	st = inb(HD2_ST);
	if(st == 0x7f) {
		puts("No Drive\n");
	} else {
		if((sc==0x01) && (sn==0x01)) {
			cl = inb(HD2_CL);
			ch = inb(HD2_CH);
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
}

