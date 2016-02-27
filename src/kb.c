//

#include "include/system.h"


#define KBD_PORT_DATA               0x60
#define KBD_PORT_STATUS             0x64
#define KBD_STAT_BUFF_FULL          0x01
#define KBD_STAT_MOUSE_BUFF_FULL    0x02

#define SCANCODE_MASK_RELEASED      0x80

#define ENABLE_AUX_MOUSE_COMMAND    0xA8
#define GET_Compaq_STATUS           0x20
#define SET_COMPAQ_STATUS           0x60

#define ACK_BYTE 0xFA

//The mouse probably sends ACK (0xFA) plus several more bytes, then resets itself, and always sends 0xAA.
#define MOUSE_CMD_RESET             0xFF

// This command makes the mouse send its most recent packet to the host again.
#define MOUSE_CMD_RESEND            0xFE

// Disables streaming, sets the packet rate to 100 per second, and resolution to 4 pixels per mm.
#define MOUSE_CMD_SET_DEFAULTS      0xF6

// The mouse stops sending automatic packets.
#define MOUSE_CMD_DISABLE_STREAMING 0xF5

// The mouse starts sending automatic packets when the mouse moves or is clicked.
#define MOUSE_CMD_ENABLE_STREAMING  0xF4

// Requires an additional data byte: automatic packets per second (see below for legal values).
#define MOUSE_CMD_SET_SAMPLE_RATE   0xF3

// The mouse sends sends its current "ID", which may change with mouse initialization.
#define MOUSE_CMD_GET_MOUSE_ID      0xF2

// The mouse sends ACK, followed by a complete mouse packet with current data.
#define MOUSE_CMD_PACKET_REQ        0xEB

// The mouse sends ACK, then 3 status bytes. See below for the status byte format.
#define MOUSE_CMD_STATUS_REQ        0xE9

// Requires an additional data byte: pixels per millimeter resolution (value 0 to 3)
#define MOUSE_CMD_RESOLUTION        0xE8


////////////////////////////////////////////////////////////////
// currently allow up to 255 characters to be buffered for use

#define MAX_BUFFERED_INPUT_KEYS 255
u8 kb_buf[MAX_BUFFERED_INPUT_KEYS] = { 0, };
i32 kb_buf_index = 0;
bool keyready = 0;

//////////////////////////////////////////////////////////////////
// Scan->Print

// http://www.quadibloc.com/comp/scan.htm
// https://www.vmware.com/support/ws55/doc/ws_devices_keymap_vscan.html

//
u8 scan_to_ascii_us[128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	  /* 9 */
    '9', '0', '-', '=', '\b', /* Backspace */
    '\t', /* Tab */
    'q', 'w', 'e', 'r', /* 19 */
    't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', /* Enter key */
    KBDUS_CONTROL, /* 29 - Control */
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', /* 39 */
    '\'', '`', KBDUS_LEFTSHIFT, /* Left shift */
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', /* 49 */
    'm', ',', '.', '/',   KBDUS_RIGHTSHIFT, /* Right shift */
    '*', KBDUS_ALT, /* Alt */
    ' ', /* Space bar */
    KBDUS_CAPSLOCK, /* Caps lock */
    KBDUS_F1, KBDUS_F2, KBDUS_F3, KBDUS_F4, KBDUS_F5,
    KBDUS_F6, KBDUS_F7, KBDUS_F8, KBDUS_F9, KBDUS_F10, /* < ... F10 */
    KBDUS_NUMLOCK, /* 69 - Num lock*/
    KBDUS_SCROLLLOCK, /* Scroll Lock */
    KBDUS_HOME, /* Home key */
    KBDUS_UPARROW, /* Up Arrow */
    KBDUS_PAGEUP, /* Page Up */
    '-',
    KBDUS_LEFTARROW, /* Left Arrow */
    0,
    KBDUS_RIGHTARROW, /* Right Arrow */
    '+',
    KBDUS_END, /* 79 - End key*/
    KBDUS_DOWNARROW, /* Down Arrow */
    KBDUS_PAGEDOWN, /* Page Down */
    KBDUS_INSERT, /* Insert Key */
    KBDUS_DELETE, /* Delete Key */
    0,   0,   0,
    KBDUS_F11, KBDUS_F12,
    0, /* All other keys are undefined */
};

//////////////////////////////////////////////////////////////////

/// Reads next character of the input stream
/// returns 0 if no key in buffer;
kbscan_t keyboard_read_next()
{
    //kbscan_t ret = 0;
    // TODO: support string formatting in serial_write
    if(--kb_buf_index < 0) {
        kb_buf_index = 0;
        return 0;
    }
    return kb_buf[kb_buf_index];
}

/* Handles the keyboard interrupt */
void keyboard_handler(isr_stack_state *r)
{
    unsigned char scancode = inb(KBD_PORT_DATA);

    // TODO: real input event system
    // - events for down, up, pressed, repeated
    // - look at how GLFW, SMFL, SDL handles this

    if (scancode & SCANCODE_MASK_RELEASED)
    {
    }
    else
    {
        kb_buf[kb_buf_index] = scancode;
        kb_buf_index++;

        enum {
            PRINT_MODE_ASCII,
            PRINT_MODE_SCAN
        };
        //u8 _curPrintMode = PRINT_MODE_ASCII;
        u8 _curPrintMode = PRINT_MODE_SCAN;
        if(scancode == KBDUS_SPACE) {
            puts("Elasped Time (in seconds): ");
            printInt( timer_seconds() );
            putch('\n');
        } else if(_curPrintMode == PRINT_MODE_SCAN) {
            printHex(scancode);
            putch('[');
            putch(scan_to_ascii_us[scancode]);
            putch(']');
        } else {
            putch(scan_to_ascii_us[scancode]);
        }
    }
}

/* Installs the keyboard handler into IRQ1 */
void keyboard_install()
{
    irq_install_handler(1, keyboard_handler);
}


/////////////////////////////////////////////////////////////////////
// Mouse
//
// http://wiki.osdev.org/Mouse_Input

//static u32 _mouse_x;
//static u32 _mouse_y;
//static u32 _mouse_x_delta;
//static u32 _mouse_y_delta;
//static u32 _mouse_b1;
//static u32 _mouse_b2; // middle
//static u32 _mouse_b3;
//static u32 _mouse_scroll_x;
//static u32 _mouse_scroll_y;
//static u32 _mouse_scroll_z;

static u8 mouse_cycle = 0;
static u32 mouse_x = 0;
static u32 mouse_y = 0;
static i8 mouse_byte[3] = { 0, }; //TODO: i think 4 would be enough??

i32 mouse_getx() { return mouse_x; }
i32 mouse_gety() { return mouse_y; }

void mouse_handler(isr_stack_state *r)
{
    // Hot Swapping:
    // When a mouse is plugged into a running system it may send a 0xAA, then a 0x00 byte
    // and then go into default state (see below).

    switch(mouse_cycle)
    {
        case 0:
            mouse_byte[0] = inb(KBD_PORT_DATA);
            mouse_cycle++;
            break;
        case 1:
            mouse_byte[1] = inb(KBD_PORT_DATA);
            mouse_cycle++;
            break;
        case 2:
            mouse_byte[2] = inb(KBD_PORT_DATA);

            //            mouse_x = mouse_byte[1];
            //            mouse_y = mouse_byte[2];

            i8 state = mouse_byte[0];
            i8 d = mouse_byte[1];
            i32 rel_x = d - ((state << 4) & 0x100);
            d = mouse_byte[2];
            i32 rel_y = d - ((state << 3) & 0x100);

            serial_writeInt(23);
            serial_write(": mouse = ");
            serial_writeInt(rel_x);
            serial_write(",");
            serial_writeInt(rel_y);
            serial_write(",");
            serial_writeHex(mouse_byte[0]);
            serial_write(",");
            serial_writeHex(mouse_byte[0]);
            serial_write(",");
            serial_writeHex(mouse_byte[0]);
            serial_write("\n");

            mouse_x += rel_x;
            mouse_y += rel_y;

            mouse_cycle = 0;
            break;
    }

    u8 status = inb(KBD_PORT_STATUS);
    while (status & KBD_STAT_BUFF_FULL)
    {
        u8 scancode = inb(KBD_PORT_DATA);
        if (status & KBD_STAT_MOUSE_BUFF_FULL) {
            UNUSED_VAR(scancode);
        }
        status = inb(KBD_PORT_STATUS);
    }
}

/// All output must be preceded by waiting for bit 1 (value=2) of port 0x64 to become clear.
/// Similarly, bytes cannot be read from port 0x60 until bit 0 (value=1) of port 0x64 is set.
static inline void mouse_wait(u8 a_type)
{
    u32 _time_out = 10000;
    if(a_type==0)
    {
        // data
        while(_time_out--)
            if((inb(KBD_PORT_STATUS) & 1)==1)
                return;
        return;
    }
    else
    {
        // signal
        while(_time_out--)
            if((inb(KBD_PORT_STATUS) & 2)==0)
                return;
        return;
    }
}

static inline void mouse_write(u8 a_write)
{
    //Wait to be able to send a command
    mouse_wait(1);
    //Tell the mouse we are sending a command
    outb(KBD_PORT_STATUS, 0xD4);
    //Wait for the final part
    mouse_wait(1);
    //Finally write
    outb(KBD_PORT_DATA, a_write);
}

/// Get's response from mouse
internal inline u8 mouse_read()
{
    mouse_wait(0);
    return inb(KBD_PORT_DATA);
}

/*
 In some systems, the PS2 aux port is disabled at boot. 
 Data coming from the aux port will not generate any interrupts. 
 To know that data has arrived, you need to enable the aux port to generate IRQ12. 
 There is only one way to do that, which involves getting/modifying the "compaq status" byte. 
 You need to send the command byte 0x20 ("Get Compaq Status Byte") to the PS2 controller on port 0x64. 
 If you look at RBIL, it says that this command is Compaq specific, but this is no longer true. 
 This command does not generate a 0xFA ACK byte. The very next byte returned should be the Status byte. 

 (Note: on some versions of Bochs, you will get a second byte, with a value of 0xD8, after sending this command, for some reason.)
 
 After you get the Status byte, you need to set bit number 1 (value=2, Enable IRQ12),
 and clear bit number 5 (value=0x20, Disable Mouse Clock). 
 Then send command byte 0x60 ("Set Compaq Status") to port 0x64, 
 followed by the modified Status byte to port 0x60. 
 This might generate a 0xFA ACK byte from the keyboard.
 */

//Get MouseID command (0xF2)

/// Install Mouse IRQ Handler
void mouse_install()
{
    u8 _status;

    //Enable the auxiliary mouse device
    mouse_wait(1);
    outb(KBD_PORT_STATUS, ENABLE_AUX_MOUSE_COMMAND);

    //Enable the interrupts
    mouse_wait(1);
    outb(KBD_PORT_STATUS, GET_Compaq_STATUS);
    mouse_wait(0);

    _status = (inb(KBD_PORT_DATA) | 2);
    mouse_wait(1);
    outb(KBD_PORT_STATUS, SET_COMPAQ_STATUS);
    mouse_wait(1);
    outb(KBD_PORT_DATA, _status);

    //Tell the mouse to use default settings
    mouse_write(MOUSE_CMD_SET_DEFAULTS);

    //Acknowledge
    mouse_read();

    //Enable the mouse
    mouse_write(MOUSE_CMD_ENABLE_STREAMING);

    //Acknowledge
    mouse_read();
    
    //Setup the mouse handler
    irq_install_handler(IRQ_MOUSE_PS2, mouse_handler);
}
