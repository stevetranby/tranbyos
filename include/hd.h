#ifndef __HD_H
#define __HD_H

#define SECTOR_BYTES		512
#define SECTOR_WORDS		256
#define SECTOR_DWORD		128 	

// could use if wanting more than one controller
#define IDE_PRIMARY		1
#define IDE_SECONDARY	0

// could use if want a programmed way to access ATA registers
#define IDE0_BASE		0x1F0
#define IDE1_BASE		0X170

#define HD0_IRQ			IRQ14
#define HD1_IRQ			IRQ15

#define HD_DATA		0x1f0			// Data port
#define HD_FEAT		0x1f1			// Features (write)
#define HD_ERR			0x1f1			// Error Info (read)
#define HD_SC			0x1f2			// Sector Count
#define HD_SN			0x1f3			// Sector Number (Low Byte of LBA - Partial Disk Sector Address)
#define HD_CL			0x1f4			// Cylinder low-byte (Mid Byte of LBA - Partial Disk Sector Address)
#define HD_CH			0x1f5			// Cylinder high-byte (High Byte of LBA - Partial Disk Sector Address)
#define HD_DH			0x1f6			// Drive select bit, 101DHHHH
#define HD_ST			0x1f7			// Status port (read)
#define HD_CMD			0x1f7			// Command port (write)
#define HD_ST_ALT		0x3f6		// Alternative Status
#define HD_DCR			0x3f6 		// Device Control Register (Alternative Status)

#define HD1_DATA			0x170			// Data port
#define HD1_FEAT			0x171			// Features (write)
#define HD1_ERR			0x171			// Error Info (read)
#define HD1_SC			0x172			// Sector Count
#define HD1_SN			0x173			// Sector Number (Low Byte of LBA - Partial Disk Sector Address)
#define HD1_CL			0x174			// Cylinder low-byte (Mid Byte of LBA - Partial Disk Sector Address)
#define HD1_CH			0x175			// Cylinder high-byte (High Byte of LBA - Partial Disk Sector Address)
#define HD1_DH			0x176			// Drive select bit, 101DHHHH
#define HD1_ST			0x177			// Status port (read)
#define HD1_CMD			0x177			// Command port (write)
#define HD1_ST_ALT		0x3f7		// Alternative Status
#define HD1_DCR			0x3f7 		// Device Control Register (Alternative Status)

#define HD_DCR_HOB	0x80			// SEt this to read back high-order byte of last LBA48 value sent to IO port.
#define HD_DCR_SRST	0x04			// Software Reset -- set this to reset all ATA drives on a bus, if one is misbehaving.
#define HD_DCR_NIEN	0x02			// Set this to stop the current device from sending interrupts.

// Bits for HD_STATUS
#define HD_ST_ERR		0x01	// Error flag (when set). Send new command to clear it (or nuke with Soft-Reset)
#define HD_ST_IDX		0x02	// ?
#define HD_ST_ECC		0x04	// corrected errors
#define HD_ST_DRQ		0x08	// Set when drive has PIO data to transfer, or is ready to accept PIO data.
#define HD_ST_SK		0x10	// Overlapped Mode Service Request (seek)
#define HD_ST_DFE		0x20	// Drive fault errors (does not set ERR!)
#define HD_ST_RDY		0x40	// Bit is clear when drive is spun down, or after an error. Set otherwise.
#define HD_ST_BSY		0x80	// drive is preparing to accept/send data -- wait until this bit clears. If it never clears, then do a soft-reset. When set other status bits are meaningless.

// Values for HD_CMD
#define HD_CMD_RESTORE		0x10	//
#define HD_CMD_READ			0x20	//
#define HD_CMD_WRITE		0x30	//
#define HD_CMD_VERIFY		0x40	//
#define HD_CMD_FORMAT		0x50	//
#define HD_CMD_INIT			0x60	//
#define HD_CMD_SEEK			0x70	//
#define HD_CMD_DIAGNOSE		0x90	//
#define HD_CMD_SPECIFY		0x91	//
#define HD_CMD_IDENTIFY		0xEC	//

// Bits for HD_ERROR
#define HD_ERR_MARK		0x01		//
#define HD_ERR_TRACK0	0x02		//
#define HD_ERR_ABORT	0x04		//
#define HD_ERR_ID		0x10		//
#define HD_ERR_ECC		0x40		//
#define HD_ERR_BBD		0x80		//


#endif

