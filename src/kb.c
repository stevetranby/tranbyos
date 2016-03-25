//

#include <system.h>

// https://github.com/klange/toaruos/blob/c4df295848fe43fb352989e3d62248c918e8542a/modules/ps2mouse.c
// Keyboard/Mouse Hardware Ports
//0x60    Read/Write  Data Port
//0x64    Read    Status Register
//0x64    Write   Command Register

#define PS2_DATA                 	0x60
#define PS2_STATUS               	0x64
#define PS2_CMD                     0x64
#define SCANCODE_MASK_RELEASED   	0x80
#define PS2_CONFIG_READ             0x20
#define PS2_CONFIG_WRITE        	0x60
#define PS2_ACK_BYTE                0xFA
#define MOUSE_WRITE              	0xD4

#define PS2_1_DISABLE 0xAD
#define PS2_2_DISABLE 0xA7
#define PS2_1_ENABLE  0xAE
#define PS2_2_ENABLE  0xA8
#define PS2_1_TEST    0xAB
#define PS2_2_TEST    0xA9

#define PS2_DEVICE_IDENTIFY             0xF2 // The mouse stops sending automatic packets.
#define PS2_DEVICE_ENABLE_STREAMING     0xF4 // The mouse starts sending automatic packets when the mouse moves or is clicked.
#define PS2_DEVICE_DISABLE_STREAMING    0xF5 // The mouse stops sending automatic packets.
#define PS2_DEVICE_RESET                0xFF // The mouse probably sends ACK (0xFA) plus several more bytes, then resets itself, and always sends 0xAA.

// Mouse 3-byte Set

#define MOUSE_DEFAULT            	0x00 //
#define MOUSE_SCROLLWHEEL        	0x01 //
#define MOUSE_BUTTONS            	0x02 //
#define MOUSE_F_BIT              	0x20 //
#define MOUSE_V_BIT              	0x08 //
#define MOUSE_CMD_RESEND            0xFE // This command makes the mouse send its most recent packet to the host again.
#define MOUSE_CMD_SET_DEFAULTS      0xF6 // Disables streaming, sets the packet rate to 100 per second, and resolution to 4 pixels per mm.
#define MOUSE_CMD_ENABLE_STREAMING  0xF4 // The mouse starts sending automatic packets when the mouse moves or is clicked.
#define MOUSE_CMD_SET_SAMPLE_RATE   0xF3 // Requires an additional data byte: automatic packets per second (see below for legal values).
#define MOUSE_CMD_GET_MOUSE_ID      0xF2 // The mouse sends sends its current "ID", which may change with mouse initialization.
#define MOUSE_CMD_PACKET_REQ        0xEB // The mouse sends ACK, followed by a complete mouse packet with current data.
#define MOUSE_CMD_STATUS_REQ        0xE9 // The mouse sends ACK, then 3 status bytes. See below for the status byte format.
#define MOUSE_CMD_RESOLUTION        0xE8 // Requires an additional data byte: pixels per millimeter resolution (value 0 to 3)

// UNUSED !!
#define SET_REMOTE_MODE				0xF0
#define SET_WRAP_MODE 				0xEE
#define RESET_WRAP_MODE				0xEC
#define SET_STREAM_MODE				0xEA

////////////////////////////////////////////////////////////////
// currently allow up to 255 characters to be buffered for use

// TODO: should scan codes buffer as u16 ?? might want to store flags?
// maybe u8 IRQ scan buf and u32 kernel scan buf
#define MAX_BUFFERED_INPUT_KEYS 255
// fixed-size ring buff using overflow to wrap around
u8 kb_buf[MAX_BUFFERED_INPUT_KEYS] = { 0, };
u8 kb_buf_first = 0; // points to first scan (not read yet)
u8 kb_buf_last = 0; // points to last scan received (LIFO)
b32 kb_buf_empty() { return kb_buf_last == kb_buf_first; }

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
    't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', /* Enter key (28) */
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
    while(kb_buf_first == kb_buf_last) {
        return 0;
    }
    return kb_buf[kb_buf_first++];
}

typedef enum {
    SUCCESS,
    ERROR_TIMEOUT = -1,
    ERROR_UNKNOWN = -2,
    ERROR_GENERIC = -3
} RETURN_CODE;

/// All output must wait for STATUS bit 1 to become clear.
static inline RETURN_CODE ps2_wait_write()
{
    u32 timeout = 100000;
    while(--timeout) {                 //
        if(! (inb(PS2_STATUS) & 0x02)) {
            return SUCCESS;
        }
    }
    trace("PS2 Timed Out!\n");
    return ERROR_TIMEOUT;
}

/// bytes cannot be read until STATUS bit 0 is set
static inline RETURN_CODE ps2_wait_read()
{
    u32 timeout = 100000;
    while(--timeout) {
        if((inb(PS2_STATUS) & 0x01)) {
            return SUCCESS;
        }

    }
    trace("PS2 Timed Out!\n");
    return ERROR_TIMEOUT;
}

// Keyboard Hardware Interrupt
// Note: we don't need to check status/avail bit with interrupts
void keyboard_handler(isr_stack_state *r)
{
    UNUSED_PARAM(r);
    //kserialf("+\n");

    //    // TODO: why does this fail? maybe reading the port itself makes it work fine?
    //    ps2_wait_read();

    u8 scancode = inb(PS2_DATA);

    // TODO: real input event system
    // - events for down, up, pressed, repeated
    // - look at how GLFW, SMFL, SDL handles this

    if (scancode & SCANCODE_MASK_RELEASED)
    {
    }
    else
    {
        kb_buf[kb_buf_last++] = scancode;

        enum {
            PRINT_MODE_ASCII,
            PRINT_MODE_SCAN
        };
        //u8 _curPrintMode = PRINT_MODE_ASCII;
        u8 _curPrintMode = PRINT_MODE_SCAN;
        if(scancode == SCAN_US_ENTER) {
            kputs("Elasped Time (in seconds): ");
            printInt( timer_seconds() );
            kputch('\n');
        } else if(scancode == SCAN_US_F2) {
            trace("\nPressed F2!\n");
            jump_usermode();
        } else if(scancode == SCAN_US_F3) {
            trace("\nPressed F3!\n");
            print_irq_counts();
        } else if(scancode == SCAN_US_F4) {
            trace("\nPressed F4!\n");
            print_blocks_avail();
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

/////////////////////////////////////////////////////////////////////
// Mouse
//
// http://wiki.osdev.org/Mouse_Input

internal mouse_device_packet mouse_packets[256];
internal u8 mouse_packets_first = 0;
internal u8 mouse_packets_last = 0;

internal u8 mouse_mode = 0;
internal u8 mouse_cycle = 0;
internal i32 mouse_x = 0;
internal i32 mouse_y = 0;
internal u16 mouse_buttons = 0;

//TODO: i think 4 would be enough??
internal u8 mouse_byte[5] = { 0, };

// TODO: Need to think about coordinate system
i32 mouse_get_x() { return mouse_x; }
i32 mouse_get_y() { return mouse_y; }
u16 mouse_get_buttons() { return mouse_buttons; }
i8 mouse_get_scrolling() { return 0; }

void add_packet(mouse_device_packet packet) {
    mouse_packets[mouse_packets_last++] = packet;
}

// TODO: need real event system (pipes, socket, slot/signal, messages)
mouse_device_packet read_next_packet() {
    mouse_device_packet p;
    if(mouse_packets_first < mouse_packets_last)
        p = mouse_packets[mouse_packets_first++];
    return p;
}

void mouse_handler(isr_stack_state *r)
{
    UNUSED_PARAM(r);

    u8 mouse_in = inb(PS2_DATA);

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
    u8 state = mouse_byte[0];
    if (state & 0x80 || state & 0x40) {
        /* x/y overflow? bad packet! */
        goto read_next;
    }

    mouse_device_packet packet;
    packet.magic = MOUSE_MAGIC;
    packet.x_difference = mouse_byte[1] - ((state << 4) & 0x100);
    packet.y_difference = -(mouse_byte[2] - ((state << 3) & 0x100));
    packet.buttons = 0;

    if (mouse_byte[0] & 0x01) {
        packet.buttons |= LEFT_CLICK;
    }

    if (mouse_byte[0] & 0x02) {
        packet.buttons |= RIGHT_CLICK;
    }

    if (mouse_byte[0] & 0x04) {
        packet.buttons |= MIDDLE_CLICK;
    }

    if (mouse_mode == MOUSE_SCROLLWHEEL && mouse_byte[3]) {
        if (mouse_byte[3] > 0) {
            packet.buttons |= MOUSE_SCROLL_DOWN;
        } else if (mouse_byte[3] < 0) {
            packet.buttons |= MOUSE_SCROLL_UP;
        }
    }

    mouse_x = CLAMP(mouse_x + packet.x_difference, 0, 600);
    mouse_y = CLAMP(mouse_y + packet.y_difference, 0, 400);

    //mouse_y += packet.y_difference;

    mouse_buttons = packet.buttons;

    add_packet(packet);

read_next:
    inb(PS2_STATUS);

    return;
}

static inline void mouse_write(u8 data)
{
    outb(PS2_CMD, MOUSE_WRITE);
    if(ps2_wait_write() == ERROR_TIMEOUT)
        return;
    outb(PS2_DATA, data);
}

/// Get's response from mouse
internal inline u8 mouse_read()
{
    if(ps2_wait_read() == ERROR_TIMEOUT)
        return 0; // no data
    return inb(PS2_DATA);
}

// TODO: combine with keyboard_install
/// Install Mouse IRQ Handler
void ps2_install()
{
    //    trace("[INFO] Not Installing Mouse!\n");
    //    return;
    trace("Installing Mouse PS/2\n");

    u8 status, result;

    // disable PS2 ports
    outb(PS2_CMD, PS2_1_DISABLE);
    outb(PS2_CMD, PS2_2_DISABLE);

    // Do a couple dummy reads from the data port.
    inb(PS2_DATA);
    inb(PS2_DATA);
    inb(PS2_DATA);

    // http://wiki.osdev.org/%228042%22_PS/2_Controller#First_PS.2F2_Port
    // 1: Initialise USB Controllers
    // 2: Determine if the PS/2 Controller Exists

    ps2_wait_write();

    // read config and set bit 2
    outb(PS2_CMD, PS2_CONFIG_READ);
    ps2_wait_read();
    status = inb(PS2_DATA);

    trace("initial config =  %b, %b\n", status, (status & 0b110111100));

    // write
    outb(PS2_CMD, PS2_CONFIG_WRITE);
    ps2_wait_write();
    outb(PS2_DATA, status & 0b110111100);

    // read config again for testing
    outb(PS2_CMD, PS2_CONFIG_READ);
    ps2_wait_read();
    status = inb(PS2_DATA);

    trace("official config = %b\n", status);

    // if clock bit for 2nd port (bit 5) is still enabled
    // assume older "single device" controller mark it unusable
    // disabled = 1, enabled = 0
    static bool is_dual_device = false;
    if(BIT(status,5)) {
        is_dual_device = true;
        trace("We believe this is a dual Device PS/2\n");
    } else {
        // don't use 2nd port
        trace("Probably a Single Device PS/2\n");
        trace("[ERR]: SHOULD NOT USE 2nd Port of PS/2\n");
    }

    outb(PS2_CMD, 0xAA);
    ps2_wait_read();
    status = inb(PS2_DATA);

    if(status == 0x55) {
        trace("PS2 Controller Test Passed\n");
    } else if(status == 0xFC) {
        trace("[err] PS2 Controller Test FAILED\n");
    } else {
        trace("[err] Unknown PS2 Controller Test Response\n");
    }

    if(is_dual_device)
    {
        // now confirm
        outb(PS2_CMD, PS2_2_ENABLE);
        // read config again for testing
        outb(PS2_CMD, PS2_CONFIG_READ);
        ps2_wait_read();
        status = inb(PS2_DATA);
        if(BIT(status,5)) {
            trace("[err] Apparently we don't actually have 2nd controller!!\n");
            is_dual_device = false;
        } else {
            trace("We have a second controller!\n");
            outb(PS2_CMD, PS2_2_DISABLE);
        }
    }

    int n = is_dual_device ? 2 : 1;

    ////////////////////////////////////////

    // TODO: At this stage, check to see how many PS/2 ports are left. If there aren't any that work you can just give up (display some errors and terminate the PS/2 Controller driver). Note: If one of the PS/2 ports on a dual PS/2 controller fails, then you can still keep going and use/support the other PS/2 port.
    // TODO: should keep track of
    // bool avail_ports[2];

    // re-enable interrupts
    outb(PS2_CMD, PS2_CONFIG_READ);
    ps2_wait_read();
    status = inb(PS2_DATA);
    status |= 0b01000001;
    if(is_dual_device)
        status |= 0b000000010;
    trace("final config = %b\n", status);
    outb(PS2_CMD, PS2_CONFIG_WRITE);
    ps2_wait_write();
    outb(PS2_DATA, status);


    // enable ports that exist
    outb(PS2_CMD, PS2_1_ENABLE);
    if(is_dual_device)
        outb(PS2_CMD, PS2_2_ENABLE);


    // disable streaming data
    for(int i=0; i < n; ++i)
    {
        if(i==1) {
            outb(PS2_CMD, MOUSE_WRITE);
        }
        ps2_wait_write();
        outb(PS2_DATA, PS2_DEVICE_DISABLE_STREAMING);
        do {
            ps2_wait_read();
            status = inb(PS2_DATA);
        } while(status != PS2_ACK_BYTE);
    }

    ////////////////////////////////////////
    // test devices
    for(int i=0; i < n; ++i)
    {
        if(i==1) {
            outb(PS2_CMD, PS2_2_TEST);
        } else {
            outb(PS2_CMD, PS2_1_TEST);
        }

        ps2_wait_read();
        status = inb(PS2_DATA);
        if(status != 0)
        {
            // error occured
            trace("[ERR] PS2 Port #%d Failed Test [code: %x!\n", i, status);
        } else {
            trace("PS2 Port #%d Passed Test Successfully\n", i);
        }
    }

    ////////////////////////////////////////
    // reset
    for(int i=0; i < n; ++i)
    {
        // reset
        if(i==1) {
            outb(PS2_CMD, MOUSE_WRITE);
        }
        ps2_wait_write();
        outb(PS2_DATA, PS2_DEVICE_RESET);

        ps2_wait_read();
        status = inb(PS2_DATA);
        if(status == 0xFA)
        {
            ps2_wait_read();
            status = inb(PS2_DATA);
            if(status == 0xAA)
            {
                trace("Reset PS2 Port #%d Successfully\n", i);
            }
        } else if(status == 0xFC) {
            trace("[ERR] Failure Resetting PS2 Port #%d\n", i);
        } else {
            trace("[ERR] Unknown error code %x Resetting PS2 Port #2\n", status, i);
        }
    }


    /*
     Identity bytes
     0xFA               AT keyboard with translation (not possible for device B)
     0xFA, 0xAB, 0x41   MF2 keyboard with translation (not possible for device B)
     0xFA, 0xAB, 0xC1   MF2 keyboard with translation (not possible for device B)
     0xFA, 0xAB, 0x83   MF2 keyboard without translation
     0xFA, 0x00         Standard mouse
     0xFA, 0x03         Mouse with a scroll wheel
     0xFA, 0x04         5 button mouse with a scroll wheel
     */

    ////////////////////////////////////////
    // identify
    static u8 ps2_identify[2][2];
    for(int i=0; i < n; ++i)
    {
        // identify
        if(i==1) {
            outb(PS2_CMD, MOUSE_WRITE);
        }
        ps2_wait_write();
        outb(PS2_DATA, PS2_DEVICE_IDENTIFY);
        do {
            ps2_wait_read();
            status = inb(PS2_DATA);
        } while(status != PS2_ACK_BYTE);

        ps2_wait_read();
        ps2_identify[i][0] = inb(PS2_DATA);
        ps2_wait_read();
        ps2_identify[i][1] = inb(PS2_DATA);
        trace("ident %d: %x,%x\n", i, ps2_identify[i][0], ps2_identify[i][1]);
    }


    // Try to enable scroll wheel (but not buttons)
    // TODO:
    if(is_dual_device)
    {
        mouse_write(MOUSE_CMD_GET_MOUSE_ID);
        result = mouse_read();
        trace("MOUSE_CMD_GET_MOUSE_ID = %d\n", result);

        // Writing Magic Mouse Settings to test for scroll wheel
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

        // TODO: this is wrong, i think (try with real corded mouse)
        trace("Set Mouse Sample Rate to 80.\n");
        mouse_write(MOUSE_CMD_GET_MOUSE_ID);
        do {
            ps2_wait_read();
            status = inb(PS2_DATA);
        } while(status != PS2_ACK_BYTE);
        result = mouse_read();
        trace("MOUSE_CMD_GET_MOUSE_ID = %d\n", result);
        if (result == 3)
        {
            trace("%x - Has Scroll Wheel.\n", result);
            mouse_mode = MOUSE_SCROLLWHEEL;
        } else {
            trace("%x - Unsure if Has Scroll Wheel.\n", result);
        }


        // Writing Magic Mouse Settings to test for scroll wheel
        mouse_write(0xF3);
        mouse_read();
        mouse_write(200);
        mouse_read();
        mouse_write(0xF3);
        mouse_read();
        mouse_write(200);
        mouse_read();
        mouse_write(0xF3);
        mouse_read();
        mouse_write(80);
        mouse_read();

        // TODO: this is wrong, i think (try with real corded mouse)
        trace("Set Mouse Sample Rate to 80.\n");
        mouse_write(MOUSE_CMD_GET_MOUSE_ID);
        do {
            ps2_wait_read();
            status = inb(PS2_DATA);
        } while(status != PS2_ACK_BYTE);
        result = mouse_read();
        trace("MOUSE_CMD_GET_MOUSE_ID = %d\n", result);
        if (result == 4) {
            trace("%x - Has 4th/5th buttons.\n", result);
            mouse_mode = MOUSE_SCROLLWHEEL;
        } else {
            trace("%x - Unsure if Has 4th/5th buttons.\n", result);
        }
    }

    //Setup the IRQ handlers

    irq_install_handler(1, keyboard_handler, "keyboard");
    trace("Keyboard handler installed.\n");

    // TODO: if has 2nd controller
    if(is_dual_device)
    {
        irq_install_handler(IRQ_MOUSE_PS2, mouse_handler, "mouse");
        trace("Installed Mouse.\n");
    }


    // re-enable streaming data
    for(int i=0; i < n; ++i)
    {
        if(i==1) {
            outb(PS2_CMD, MOUSE_WRITE);
        }
        ps2_wait_write();
        outb(PS2_DATA, PS2_DEVICE_ENABLE_STREAMING);
        do {
            ps2_wait_read();
            status = inb(PS2_DATA);
        } while(status != PS2_ACK_BYTE);
    }
}
