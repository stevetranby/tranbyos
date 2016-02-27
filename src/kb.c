#include "include/system.h"

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
    unsigned char scancode;

    /* Read from the keyboard's data buffer */
    static const u8 PORT_KEYBOARD_DATA = 0x60;
    scancode = inb(PORT_KEYBOARD_DATA);

    // TODO: real input event system
    // - events for down, up, pressed, repeated
    // - look at how GLFW, SMFL, SDL handles this

    static const u8 SCANCODE_MASK_RELEASED = 0x80;
    if (scancode & SCANCODE_MASK_RELEASED)
    {
        /* You can use this one to see if the user released the
         *  shift, alt, or control keys... */
    }
    else
    {
        kb_buf[kb_buf_index] = scancode;//scan_to_ascii_us[scancode];
        kb_buf_index++;

        enum {
            PRINT_MODE_ASCII,
            PRINT_MODE_SCAN
        };
        //u8 _curPrintMode = PRINT_MODE_ASCII;
        u8 _curPrintMode = PRINT_MODE_SCAN;
        //u8 KBDUS_SPACE = 0x39;
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

#define KBD_STAT_OUT_BUFF_FULL 0x01
#define KBD_STAT_MOUSE_OUT_BUFF_FULL 0x02

#define KBD_PORT_DATA 0x60
#define KBD_PORT_STATUS 0x64

i32 mouse_getx() { return mouse_x; }
i32 mouse_gety() { return mouse_y; }

void mouse_handler(isr_stack_state *r)
{
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

            serial_printInt(23);
            serial_write(": mouse = ");
            serial_printInt(rel_x);
            serial_write(",");
            serial_printInt(rel_y);
            serial_write("\n");

            mouse_x += rel_x;
            mouse_y += rel_y;

            mouse_cycle = 0;
            break;
    }

    u8 status = inb(KBD_PORT_STATUS);
    while (status & KBD_STAT_OUT_BUFF_FULL)
    {
        u8 scancode = inb(KBD_PORT_DATA);
        if (status & KBD_STAT_MOUSE_OUT_BUFF_FULL) {
            UNUSED_VAR(scancode);
        }
        status = inb(KBD_PORT_STATUS);
    }
    //printHex(status);

#ifdef DEBUG
    //    puts("{");
    //    printInt(mouse_x);
    //    puts(",");
    //    printInt(mouse_y);
    //    puts(",");
    //    printHex(mouse_byte[0]);
    //    puts(",");
    //    printHex(mouse_byte[1]);
    //    puts(",");
    //    printHex(mouse_byte[2]);
    //    puts("} ");
#endif
}

static inline void mouse_wait(u8 a_type)
{
    u32 _time_out = 10000;
    if(a_type==0)
    {
        // data
        while(_time_out--)
        {
            if((inb(0x64) & 1)==1)
            {
                return;
            }
        }
        return;
    }
    else
    {
        // signal
        while(_time_out--)
        {
            if((inb(0x64) & 2)==0)
            {
                return;
            }
        }
        return;
    }
}

static inline void mouse_write(u8 a_write)
{
    //Wait to be able to send a command
    mouse_wait(1);
    //Tell the mouse we are sending a command
    outb(0x64, 0xD4);
    //Wait for the final part
    mouse_wait(1);
    //Finally write
    outb(0x60, a_write);
}

u8 mouse_read()
{
    //Get's response from mouse
    mouse_wait(0);
    return inb(0x60);
}

// Install IRQ Handler
void mouse_install()
{
    u8 _status;

    //Enable the auxiliary mouse device
    mouse_wait(1);
    outb(0x64, 0xA8);

    //Enable the interrupts
    mouse_wait(1);
    outb(0x64, 0x20);
    mouse_wait(0);
    _status = (inb(0x60) | 2);
    mouse_wait(1);
    outb(0x64, 0x60);
    mouse_wait(1);
    outb(0x60, _status);

    //Tell the mouse to use default settings
    mouse_write(0xF6);
    
    //Acknowledge
    mouse_read();

    //Enable the mouse
    mouse_write(0xF4);
    
    //Acknowledge
    mouse_read();
    
    //Setup the mouse handler
    irq_install_handler(IRQ_MOUSE_PS2, mouse_handler);
}
