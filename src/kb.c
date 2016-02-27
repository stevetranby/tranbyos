//

#include "include/system.h"


#define KBD_PORT               0x60
#define KBD_STATUS             0x64
#define MOUSE_PORT               0x60
#define MOUSE_STATUS             0x64

#define KBD_STAT_BUFF_FULL          0x01
#define KBD_STAT_MOUSE_BUFF_FULL    0x02

#define SCANCODE_MASK_RELEASED      0x80

#define ENABLE_AUX_MOUSE_COMMAND    0xA8
#define GET_COMPAQ_STATUS           0x20
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
    unsigned char scancode = inb(KBD_PORT);

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


internal u8 mouse_mode = 0;
internal u8 mouse_cycle = 0;
internal u32 mouse_x = 0;
internal u32 mouse_y = 0;
internal i8 mouse_byte[3] = { 0, }; //TODO: i think 4 would be enough??
//internal bool mouse_button[5];

i32 mouse_getx() { return mouse_x; }
i32 mouse_gety() { return mouse_y; }

// https://github.com/klange/toaruos/blob/c4df295848fe43fb352989e3d62248c918e8542a/modules/ps2mouse.c
#define MOUSE_WRITE  0xD4
#define MOUSE_PORT   0x60
#define MOUSE_STATUS 0x64
#define MOUSE_ABIT   0x02
#define MOUSE_BBIT   0x01
#define MOUSE_WRITE  0xD4
#define MOUSE_F_BIT  0x20
#define MOUSE_V_BIT  0x08


#define MOUSE_DEFAULT 0
#define MOUSE_SCROLLWHEEL 1
#define MOUSE_BUTTONS 2

void mouse_handler(isr_stack_state *r)
{
    u8 status = inb(MOUSE_STATUS);
    while (status & 0x02)
    {
        u8 mouse_in = inb(MOUSE_PORT);
        if(status & 0x20)
        {
            switch (mouse_cycle) {
                case 0:
                    mouse_byte[0] = mouse_in;
                    if (!(mouse_in & 0x08)) {
                        goto read_next;
                    }
                    ++mouse_cycle;
                    break;
                case 1:
                    mouse_byte[1] = mouse_in;
                    ++mouse_cycle;
                    break;
                case 2:
                    mouse_byte[2] = mouse_in;
                    if (mouse_mode == MOUSE_SCROLLWHEEL || mouse_mode == MOUSE_BUTTONS) {
                        ++mouse_cycle;
                        break;
                    }
                    goto finish_packet;
                case 3:
                    mouse_byte[3] = mouse_in;
                    goto finish_packet;
            }

            goto read_next;


        finish_packet:

            mouse_cycle = 0;
            if (mouse_byte[0] & 0x80 || mouse_byte[0] & 0x40) {
                /* x/y overflow? bad packet! */

                goto read_next;
            }
            /* We now have a full mouse packet ready to use */
//            mouse_device_packet_t packet;
//            packet.magic = MOUSE_MAGIC;
//            packet.x_difference = mouse_byte[1];
//            packet.y_difference = mouse_byte[2];
//            packet.buttons = 0;

            if (mouse_byte[0] & 0x01) {
//                packet.buttons |= LEFT_CLICK;
            }

            if (mouse_byte[0] & 0x02) {
//                packet.buttons |= RIGHT_CLICK;
            }

            if (mouse_byte[0] & 0x04) {
//                packet.buttons |= MIDDLE_CLICK;
            }
            
            if (mouse_mode == MOUSE_SCROLLWHEEL && mouse_byte[3]) {
                if (mouse_byte[3] > 0) {
//                    packet.buttons |= MOUSE_SCROLL_DOWN;
                } else if (mouse_byte[3] < 0) {
//                    packet.buttons |= MOUSE_SCROLL_UP;
                }
            }

            serial_writeInt(23);
            serial_write(": mouse = ");
            serial_writeBinary_b(mouse_byte[0]);
            serial_write(", ");
            serial_writeHex_b(mouse_byte[1]);
            serial_write(", ");
            serial_writeHex_b(mouse_byte[2]);
            serial_write("\n");


//            mouse_device_packet_t bitbucket;
//            while (pipe_size(mouse_pipe) > (int)(DISCARD_POINT * sizeof(packet))) {
//                read_fs(mouse_pipe, 0, sizeof(packet), (uint8_t *)&bitbucket);
//            }
//            write_fs(mouse_pipe, 0, sizeof(packet), (uint8_t *)&packet);
        }
    read_next:
        status = inb(MOUSE_STATUS);
    }

    return;


//    // Hot Swapping:
//    // When a mouse is plugged into a running system it may send a 0xAA, then a 0x00 byte
//    // and then go into default state (see below).
//
//    switch(mouse_cycle)
//    {
//        case 0:
//            mouse_byte[0] = inb(MOUSE_PORT);
//            mouse_cycle++;
//            break;
//        case 1:
//            mouse_byte[1] = inb(MOUSE_PORT);
//            mouse_cycle++;
//            break;
//        case 2:
//            mouse_byte[2] = inb(MOUSE_PORT);
//
//            // The top two bits of the first byte (values 0x80 and 0x40) supposedly show Y and X overflows.
//            // They are not useful. If they are set, you should probably just discard the entire packet.
//
//            i8 state = mouse_byte[0];
//            i32 dx = mouse_byte[1];
//            i32 dy = mouse_byte[2];
//
//            if(state & 0x10) {
//                // dx is negative
//                dx = dx & 0xffffff00;
//            }
//
//            if(state & 0x20) {
//                // dy is negative
//                dx = dx & 0xffffff00;
//            }
//
//            if(0 == (state & 0x08)) {
//                // invalid packed
//                serial_write("INVALID PACKED\n");
//            }
//            if(0 == (state & 0x40)) {
//                serial_write("X Overflow!\n");
//            }
//            if(0 == (state & 0x80)) {
//                serial_write("Y Overflow!\n");
//            }
//
//            // bits 0-4 are the buttons left(0),right(1),middle(2),4th(3),5th(4)
//            mouse_button[0] = state & 0x01;
//            mouse_button[1] = state & 0x02;
//            mouse_button[2] = state & 0x03;
//
//            serial_writeInt(23);
//            serial_write(": mouse = ");
//            serial_writeInt(dx);
//            serial_write(", ");
//            serial_writeInt(dy);
//            serial_write(" ::\t");
//            serial_writeBinary_b(mouse_byte[0]);
//            serial_write(", ");
//            serial_writeHex_b(mouse_byte[1]);
//            serial_write(", ");
//            serial_writeHex_b(mouse_byte[2]);
//            serial_write("\n");
//
//            mouse_x += dx;
//            mouse_y += dy;
//
//            mouse_cycle = 0;
//            break;
//    }
//
//    status = inb(MOUSE_STATUS);
//    while (status & KBD_STAT_BUFF_FULL)
//    {
//        u8 scancode = inb(MOUSE_PORT);
//        if (status & KBD_STAT_MOUSE_BUFF_FULL) {
//            UNUSED_VAR(scancode);
//        }
//        status = inb(MOUSE_STATUS);
//    }
}

#define MOUSE_BIT_1 1
#define MOUSE_BIT_2 2
#define MOUSE_WRITE 0xD4

/// All output must be preceded by waiting for bit 1 (value=2) of port 0x64 to become clear.
/// Similarly, bytes cannot be read from port 0x60 until bit 0 (value=1) of port 0x64 is set.
static inline void mouse_wait(u8 a_type)
{
    u32 timeout = 100000;

    if(! a_type) {
        while(--timeout) {
            if((inb(MOUSE_STATUS) & MOUSE_BIT_1) == 1) {
                return;
            }
        }
        puts("Mouse Timed OUt!");
        return;
    } else {
        while(--timeout) {
            if(! (inb(MOUSE_STATUS) & MOUSE_BIT_2)) {
                return;
            }

        }
        puts("Mouse Timed OUt!");
        return;
    }
}


static inline void mouse_write(u8 a_write)
{
    mouse_wait(1);
    outb(MOUSE_STATUS, MOUSE_WRITE);
    mouse_wait(1);
    outb(MOUSE_PORT, a_write);
}

/// Get's response from mouse
internal inline u8 mouse_read()
{
    mouse_wait(0);
    return inb(MOUSE_PORT);
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
    u8 status, result;

    puts("Installing Mouse PS/2");

    //Enable the auxiliary mouse device
    mouse_wait(1);
    outb(MOUSE_STATUS, ENABLE_AUX_MOUSE_COMMAND);

    //Enable the interrupts
    mouse_wait(1);
    outb(MOUSE_STATUS, GET_COMPAQ_STATUS);
    mouse_wait(0);

    status = inb(MOUSE_PORT) | 2;
    mouse_wait(1);
    outb(MOUSE_STATUS, SET_COMPAQ_STATUS);
    mouse_wait(1);
    outb(MOUSE_PORT, status);

    //Tell the mouse to use default settings
    mouse_write(MOUSE_CMD_SET_DEFAULTS);
    mouse_read();
    mouse_write(MOUSE_CMD_ENABLE_STREAMING);
    mouse_read();

    // Try to enable scroll wheel (but not buttons)
    {
        mouse_write(0xF2);
        mouse_read();
        result = mouse_read();
        mouse_write(0xF3);
        mouse_read();
        mouse_write(200);
        mouse_read();
        mouse_write(0xF3);
        mouse_read();
        mouse_write(100);
        mouse_read();
        mouse_write(0xF3);
        mouse_read();
        mouse_write(80);
        mouse_read();
        mouse_write(0xF2);
        mouse_read();
        result = mouse_read();
        if (result == 3) {
            serial_write("Has Scroll Wheel.");
            mouse_mode = MOUSE_SCROLLWHEEL;
        }
    }

    //Setup the mouse handler
    irq_install_handler(IRQ_MOUSE_PS2, mouse_handler);

    //////////////////////////////////////

    u8 tmp = inb(0x61);
    outb(0x61, tmp | 0x80);
    outb(0x61, tmp & 0x7F);
    inb(MOUSE_PORT);

    serial_write("Installed Mouse.");
}
