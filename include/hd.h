#ifndef __HD_H
#define __HD_H

#define HD_BASE		0x1F0
#define HD2_BAS		0X170

// #define HD_IRQ			IRQ14
// #define HD2_IRQ		IRQ15

#define HD_DATA		0x1f0			// Data port
#define HD_FEATURES	0x1f1			// Features (write)
#define HD_ERR			0x1f1			// Error Info (read)
#define HD_NSECTOR	0x1f2			// Sector Count
#define HD_SECTOR		0x1f3			// Partial Disk Sector Address
#define HD_LOWCYL		0x1f4			// Partial Disk Sector Address
#define HD_HIGHCYL	0x1f5			// Partial Disk Sector Address
#define HD_SELECT		0x1f6			// Drive select bit, Flag bits, Extra address bits
#define HD_STATUS		0x1f7			// Status port (read)
#define HD_CMD			0x1f7			// Command port (write)
#define HD_DCR			0x3f6 		// Device Control Register (Alternative Status)

#define HD2_DATA		0x170			// Data port
#define HD2_FEATURES	0x171			// Features (write)
#define HD2_ERR		0x171			// Error Info (read)
#define HD2_NSECTOR	0x172			// Sector Count
#define HD2_SECTOR	0x173			// Partial Disk Sector Address
#define HD2_LOWCYL	0x174			// Partial Disk Sector Address
#define HD2_HIGHCYL	0x175			// Partial Disk Sector Address
#define HD2_SELECT	0x176			// Drive select bit, Flag bits, Extra address bits
#define HD2_STATUS	0x177			// Status port (read)
#define HD2_CMD		0x177			// Command port (write)
#define HD2_DCR		0x3f7 		// Secondary Bus DCR

#define HD_DCR_HOB	0x80			// SEt this to read back high-order byte of last LBA48 value sent to IO port.
#define HD_DCR_SRST	0x04			// Software Reset -- set this to reset all ATA drives on a bus, if one is misbehaving.
#define HD_DCR_NIEN	0x02			// Set this to stop the current device from sending interrupts.

// Bits for HD_STATUS
#define HD_STATUS_ERR		0x01	// Error flag (when set). Send new command to clear it (or nuke with Soft-Reset)
#define HD_STATUS_IDX		0x02	// ?
#define HD_STATUS_ECC		0x04	// corrected errors
#define HD_STATUS_DRQ		0x08	// Set when drive has PIO data to transfer, or is ready to accept PIO data.
#define HD_STATUS_SRV		0x10	// Overlapped Mode Service Request
#define HD_STATUS_DF			0x20	// Drive fault errors (does not set ERR!)
#define HD_STATUS_RDY		0x40	// Bit is clear when drive is spun down, or after an error. Set otherwise.
#define HD_STATUS_BSY		0x80	// drive is preparing to accept/send data -- wait until this bit clears. If it never clears, then do a soft-reset. When set other status bits are meaningless.

// Values for HD_CMD
#define HD_CMD_RESTORE		0x10	//
#define HD_CMD_READ			0x20	//
#define HD_CMD_WRITE			0x30	//
#define HD_CMD_VERIFY		0x40	//
#define HD_CMD_FORMAT		0x50	//
#define HD_CMD_INIT			0x60	//
#define HD_CMD_SEEK			0x70	//
#define HD_CMD_DIAGNOSE		0x90	//
#define HD_CMD_SPECIFY		0x91	//

// Bits for HD_ERROR
#define HD_ERR_MARK		0x01		//
#define HD_ERR_TRACK0	0x02		//
#define HD_ERR_ABORT		0x04		//
#define HD_ERR_ID			0x10		//
#define HD_ERR_ECC		0x40		//
#define HD_ERR_BBD		0x80		//

#endif
