#include <system.h>
#include <hd.h>


int wait_busy(void) {
	while((inb(HD_ST) & HD_ST_BSY)) ;
}

int is_ready(void) {
	if((inb(HD_ST)&(HD_ST_BSY|HD_ST_DRQ))==0) return 1;
	return 0;
}

int reset_controller(void) {
	int i;	
	outb(HD_ST, 0x04);	
	for(i=0;i<10000;++i) nop();
	outb(HD_ST, 0x00);	
	for(i=0;i<10000;++i) if(is_ready()) break;
	if(!is_ready()) return 1;
	return 0; /* okay */	
}

int select_device(uint device) {
	int i;
	if(device>1) return 0;
	for(i=0;i<10000;++i) if(is_ready()) break;
	if(!is_ready()) return 0;
	puts("Writing "); printHex(0x80|0x20|0); putch('\n');
	outb(HD_DH, (0xA0|(device<<4)));
	for(i=0;i<10000;++i) if(is_ready()) break;
	if(!is_ready()) return 0;
	return 1;
}

// issue software reset
int reset_devices(void)
{
    // write status
    outb(HD_ST, 0x02);
    timer_wait(5);
    outb(HD_DCR, HD_DCR_SRST);
    timer_wait(5);

    // If there is a device 0, wait for device 0 to set BSY=0
    int t;
    for(t=0; t<99999; ++t)
    {
        if((inb(HD_ST) & HD_ST_BSY) == 0)
            break;
    }
    if(t==99999)
        puts("Device 0 reset timeout\n");

    return 0;
}

/*
static int drive_busy(void) {
	uint32 i;	
	for(i=0; i<10000; ++i) 
		if(HD_ST_RDY == (inb(HD_ST) & (HD_ST_BSY | HD_ST_RDY)))
			break;
			
	i = inb(HD_ST);
	i &= HD_ST_BSY | HD_ST_RDY | HD_ST_SK;
	if(i == (HD_ST_RDY | HD_ST_SK))
		return (0);
	puts("HD Controller times out\n");
	return(1);	
}
*/

/*
static void reset_hd(int nr) {
	reset_controller();
	hd_out(0,1,1,0,0,HD_CMD_SPECIFY);
}
*/

/*
static void hd_out(int drive, int sc, int sn, int head, int cyl, int cmd)
{
	register int port asm("dx");
	
	if(!controller_ready()) {
		puts("Controller Not Ready");
		return;
	}
	
	outb(HD_CMD, cmd);
	port=HD_DATA;
	outb(HD_SC, sc);
	outb(HD_SN, sn);
	outb(HD_CL, 0x00);
	outb(HD_CH, 0x00);
	outb(HD_DH, 0xa0 | (drive << 4));
}
*/

void hd_print_status(void)
{
    uint8 status = inb(0x3F6); // alternate status port

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
    for(i=0;i<8;++i)
        putch('\b');
}

int hd_write_b(uint32 sn, byte *data, int n, uint8 slave)
{
	int i;
	
	for(i=0;i<10000;++i) if(is_ready()) break;
	if(!is_ready()) return 0;
	if(!select_device(0)) return 0; // selecting master
	
    outb(HD_SC, 1); // sector count port, read 1 sector
    outb(HD_SN, 1); // sector # port, read sector 1
    outb(HD_CL, 0); // cyl low port, cyl 0
    outb(HD_CH, 0); // cyl high port, rest of cyl 0
    outb(HD_DH, (0x80|0x20));
    outb(HD_CMD, HD_CMD_WRITE); // cmd port, read with retry

	timer_wait(10);

	while(((inb(HD_ST) & (HD_ST_BSY|HD_ST_DRQ)) != HD_ST_DRQ) || (inb(HD_ST) & HD_ST_ERR)) {}
    if(inb(HD_ST) & HD_ST_ERR) return 0;	

	// write out data
    for(i=0; i<SECTOR_BYTES; ++i) outb(HD_DATA, data[i]);
    
    
    
    return 1;
}

int hd_read_b(uint32 sn, uint32 sc, uint8 slave)
{
	if(sn < 1) return 0;
	
    // read the bytes of this sector
    byte data[sc*SECTOR_BYTES];
    int cl = 0;
    int ch = 0;
    int head = 0;

    // prime the controller
    outb(HD_ERR, 0x0); // Reset Status    
    outb(HD_SC, sc); // sector count port, read 1 sector
    outb(HD_SN, sn); // sector # port, read sector 1
    outb(HD_CL, cl); // cyl low port, cyl 0
    outb(HD_CH, ch); // cyl high port, rest of cyl 0
    outb(HD_DH, 0xA0 | (slave << 4) | ((head) & 0x0f)); // select device 101dhhhh
    outb(HD_CMD, HD_CMD_READ); // cmd port, read with retry

    // Wait 
    while(!(inb(HD_ST) & HD_ST_DRQ)) {}
    
    int i;
    for(i=0; i<(sc*SECTOR_BYTES); ++i)
    {
        data[i] = inb(HD_DATA);
        printHex(data[i]);        
    }
    
    return 0;
}

/* on Primary bus: ctrl->base =0x1F0, ctrl->dev_ctl =0x3F6. REG_CYL_LO=4, REG_CYL_HI=5, REG_DEVSEL=6 */
void print_hd_device_types()
{
    byte sc, sn, st, cl, ch;

    outb(HD_ST, 0x08);
    outb(HD_DH, 0xa0);
    timer_wait(5);
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
    timer_wait(5);
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
    timer_wait(5);

    sc = inb(HD_SC);
    sn = inb(HD_SN);
    st = inb(HD_ST);
    if(st == 0x7f)
    {
        puts("No Drive\n");
    }
    else
    {
        if((sc==0x01) && (sn==0x01))
        {
            cl = inb(HD_CL);
            ch = inb(HD_CH);
            if((cl==0x14)&&(ch=0xeb))
                puts("Primary Master is ATAPI\n");
            else if ((cl==0x00) && (ch==0x00))
                puts("Primary Master is ATA\n");
            else
            {
                puts("Identify drive: cl=");
                printHex(cl);
                puts(" ch=");
                printHex(ch);
                putch('\n');
            }
        }
        else
        {
            puts("Identifying drive: sc=");
            printHex(sc);
            puts(" sn=");
            printHex(sn);
            putch('\n');
        }
    }

    outb(HD_DH, 0xb0);
    timer_wait(5);

    sc = inb(HD_SC);
    sn = inb(HD_SN);
    st = inb(HD_ST);
    if(st == 0x7f)
    {
        puts("No Drive\n");
    }
    else
    {
        if((sc==0x01) && (sn==0x01))
        {
            cl = inb(HD_CL);
            ch = inb(HD_CH);
            if((cl==0x14)&&(ch=0xeb))
                puts("Primary Slave is ATAPI\n");
            else if ((cl==0x00) && (ch==0x00))
                puts("Primary Slave is ATA\n");
            else
            {
                puts("Identify drive: cl=");
                printHex(cl);
                puts(" ch=");
                printHex(ch);
                putch('\n');
            }
        }
        else
        {
            puts("Identifying drive: sc=");
            printHex(sc);
            puts(" sn=");
            printHex(sn);
            putch('\n');
        }
    }


    // Soft Reset
    reset_devices();

    // 2nd IDE Controller
    outb(HD2_ST, 0x08);
    outb(HD2_DH, 0xa0);
    timer_wait(5);
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
    timer_wait(5);
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
    timer_wait(5);

    sc = inb(HD2_SC);
    sn = inb(HD2_SN);
    st = inb(HD2_ST);
    if(st == 0x7f)
    {
        puts("No Drive\n");
    }
    else
    {
        if((sc==0x01) && (sn==0x01))
        {
            cl = inb(HD2_CL);
            ch = inb(HD2_CH);
            if((cl==0x14)&&(ch=0xeb))
                puts("Secondary Master is ATAPI\n");
            else if ((cl==0x00) && (ch==0x00))
                puts("Secondary Master is ATA\n");
            else
            {
                puts("Identify drive: cl=");
                printHex(cl);
                puts(" ch=");
                printHex(ch);
                putch('\n');
            }
        }
        else
        {
            puts("Identifying drive: sc=");
            printHex(sc);
            puts(" sn=");
            printHex(sn);
            putch('\n');
        }
    }

    outb(HD_DH, 0xb0);
    timer_wait(5);

    sc = inb(HD2_SC);
    sn = inb(HD2_SN);
    st = inb(HD2_ST);
    if(st == 0x7f)
    {
        puts("No Drive\n");
    }
    else
    {
        if((sc==0x01) && (sn==0x01))
        {
            cl = inb(HD2_CL);
            ch = inb(HD2_CH);
            if((cl==0x14)&&(ch=0xeb))
                puts("Secondary Slave is ATAPI\n");
            else if ((cl==0x00) && (ch==0x00))
                puts("Secondary Slave is ATA\n");
            else
            {
                puts("Identify drive: cl=");
                printHex(cl);
                puts(" ch=");
                printHex(ch);
                putch('\n');
            }
        }
        else
        {
            puts("Identifying drive: sc=");
            printHex(sc);
            puts(" sn=");
            printHex(sn);
            putch('\n');
        }
    }
}

