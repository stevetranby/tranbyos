//

#include <system.h>

// https://github.com/klange/toaruos/blob/c4df295848fe43fb352989e3d62248c918e8542a/modules/ps2mouse.c
// Keyboard/Mouse Hardware Ports
#define PS2_PORT                 	0x60
#define PS2_STATUS               	0x64
#define SCANCODE_MASK_RELEASED   	0x80
#define ENABLE_AUX_MOUSE_COMMAND 	0xA8
#define GET_COMPAQ_STATUS        	0x20
#define SET_COMPAQ_STATUS        	0x60
#define ACK_BYTE                 	0xFA
#define MOUSE_WRITE              	0xD4

// Mouse 3-byte Set
#define MOUSE_DEFAULT            	0x00 //
#define MOUSE_SCROLLWHEEL        	0x01 //
#define MOUSE_BUTTONS            	0x02 //
#define MOUSE_F_BIT              	0x20 //
#define MOUSE_V_BIT              	0x08 //
#define MOUSE_CMD_RESET             0xFF //The mouse probably sends ACK (0xFA) plus several more bytes, then resets itself, and always sends 0xAA.
#define MOUSE_CMD_RESEND            0xFE // This command makes the mouse send its most recent packet to the host again.
#define MOUSE_CMD_SET_DEFAULTS      0xF6 // Disables streaming, sets the packet rate to 100 per second, and resolution to 4 pixels per mm.
#define MOUSE_CMD_DISABLE_STREAMING 0xF5 // The mouse stops sending automatic packets.
#define MOUSE_CMD_ENABLE_STREAMING  0xF4 // The mouse starts sending automatic packets when the mouse moves or is clicked.
#define MOUSE_CMD_SET_SAMPLE_RATE   0xF3 // Requires an additional data byte: automatic packets per second (see below for legal values).
#define MOUSE_CMD_GET_MOUSE_ID      0xF2 // The mouse sends sends its current "ID", which may change with mouse initialization.
#define MOUSE_CMD_PACKET_REQ        0xEB // The mouse sends ACK, followed by a complete mouse packet with current data.
#define MOUSE_CMD_STATUS_REQ        0xE9 // The mouse sends ACK, then 3 status bytes. See below for the status byte format.
#define MOUSE_CMD_RESOLUTION        0xE8 // Requires an additional data byte: pixels per millimeter resolution (value 0 to 3)

////////////////////////////////////////////////////////////////
// currently allow up to 255 characters to be buffered for use

#define MAX_BUFFERED_INPUT_KEYS 255
u8 kb_buf[MAX_BUFFERED_INPUT_KEYS] = { 0, };
i32 kb_buf_index = 0;
b32 keyready = 0;

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

// TODO: convert into ring buffer impl
/// Reads next character of the input stream
/// returns 0 if no key in buffer;
kbscan_t keyboard_read_next()
{
    // TODO: support string formatting in serial_write
    if(--kb_buf_index < 0) {
        kb_buf_index = 0;
        return 0;
    }
    return kb_buf[kb_buf_index];
}

// TODO: look at if this should be PS/2 wait instead

#define PS2_STATUS 0x64

/// All output must wait for STATUS bit 1 to become clear.
static inline void ps2_wait_write()
{
    u32 timeout = 100000;
    while(--timeout) {                 //
        if(inb(PS2_STATUS) & 0x01) {
            return;
        }
    }
    trace("PS2 Timed Out!");
    return;
}

/// bytes cannot be read until STATUS bit 0 is set
static inline void ps2_wait_read()
{
    u32 timeout = 100000;
    while(--timeout) {
        if(! (inb(PS2_STATUS) & 0x02)) {
            return;
        }

    }
    trace("PS2 Timed Out!");
    return;
}

// Keyboard Hardware Interrupt
void keyboard_handler(isr_stack_state *r)
{
    UNUSED_PARAM(r);
    serial_write("+");

    // TODO: why does this fail? maybe reading the port itself makes it work fine?
    ps2_wait_read();

    u8 scancode = inb(PS2_PORT);

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
        if(scancode == SCAN_US_SPACE) {
            kputs("Elasped Time (in seconds): ");
            printInt( timer_seconds() );
            kputch('\n');
        } else if(scancode == SCAN_US_F2) {
            kputs("\nPressed F2!\n");
            print_irq_counts();
        } else if(_curPrintMode == PRINT_MODE_SCAN) {
            printHex(scancode);
            kputch('[');
            kputch(scan_to_ascii_us[scancode]);
            kputch(']');
        } else {
            kputch(scan_to_ascii_us[scancode]);
        }
    }
}

/* Installs the keyboard handler into IRQ1 */
void keyboard_install()
{
    serial_write("Installing Mouse PS/2\n");
    irq_install_handler(1, keyboard_handler, "keyboard");
    serial_write("Keyboard handler installed.\n");
}


/////////////////////////////////////////////////////////////////////
// Mouse
//
// http://wiki.osdev.org/Mouse_Input


internal u8 mouse_mode = 0;
internal u8 mouse_cycle = 0;
internal u32 mouse_x = 0;
internal u32 mouse_y = 0;
internal i8 mouse_byte[5] = { 0, }; //TODO: i think 4 would be enough??
//internal bool mouse_button[5];

i32 mouse_getx() { return mouse_x; }
i32 mouse_gety() { return mouse_y; }

void mouse_handler(isr_stack_state *r)
{
    UNUSED_PARAM(r);
    serial_write("^");
    
    u8 status = inb(PS2_STATUS);
    while (status & 0x02)
    {
        u8 mouse_in = inb(PS2_PORT);
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
        status = inb(PS2_STATUS);
    }

    return;


//    // Hot Swapping:
//    // When a mouse is plugged into a running system it may send a 0xAA, then a 0x00 byte
//    // and then go into default state (see below).
//
//    switch(mouse_cycle)
//    {
//        case 0:
//            mouse_byte[0] = inb(PS2_PORT);
//            mouse_cycle++;
//            break;
//        case 1:
//            mouse_byte[1] = inb(PS2_PORT);
//            mouse_cycle++;
//            break;
//        case 2:
//            mouse_byte[2] = inb(PS2_PORT);
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
//    status = inb(PS2_STATUS);
//    while (status & KBD_STAT_BUFF_FULL)
//    {
//        u8 scancode = inb(PS2_PORT);
//        if (status & KBD_STAT_MOUSE_BUFF_FULL) {
//            UNUSED_VAR(scancode);
//        }
//        status = inb(PS2_STATUS);
//    }
}

#define MOUSE_WRITE 0xD4

static inline void mouse_write(u8 data)
{
    ps2_wait_write();
    outb(PS2_STATUS, MOUSE_WRITE);
    ps2_wait_write();
    outb(PS2_PORT, data);
}

/// Get's response from mouse
internal inline u8 mouse_read()
{
    ps2_wait_read();
    return inb(PS2_PORT);
}

/// Install Mouse IRQ Handler
void mouse_install()
{
    u8 status, result;

    serial_write("Installing Mouse PS/2\n");

    //Enable the auxiliary mouse device
    ps2_wait_write();
    outb(PS2_STATUS, ENABLE_AUX_MOUSE_COMMAND);

    //Enable the interrupts
    ps2_wait_write();
    outb(PS2_STATUS, GET_COMPAQ_STATUS);

    ps2_wait_read();
    status = inb(PS2_PORT) | 2;

    ps2_wait_write();
    outb(PS2_STATUS, SET_COMPAQ_STATUS);

    ps2_wait_write();
    outb(PS2_PORT, status);

    //Tell the mouse to use default settings
    mouse_write(MOUSE_CMD_SET_DEFAULTS);
    mouse_read();
    mouse_write(MOUSE_CMD_ENABLE_STREAMING);
    mouse_read();

    // Try to enable scroll wheel (but not buttons)
    {
        mouse_write(MOUSE_CMD_GET_MOUSE_ID);
        mouse_read();
        result = mouse_read();

        // Writing Mouse Settings (TODO: 200,100,80 ???)
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
            serial_write("Has Scroll Wheel.\n");
            mouse_mode = MOUSE_SCROLLWHEEL;
        }
    }

    //Setup the mouse handler
    irq_install_handler(IRQ_MOUSE_PS2, mouse_handler, "mouse");

    //////////////////////////////////////

    // TOOD: what is this??
    u8 tmp = inb(0x61);
    outb(0x61, tmp | 0x80);
    outb(0x61, tmp & 0x7F);
    inb(PS2_PORT);

    serial_write("Installed Mouse.\n");
}
