#include "include/system.h"
#include "include/kb.h"

// currently allow up to 255 characters to be buffered for use
#define MAX_BUFFERED_INPUT_KEYS 255
u8 kb_buf[MAX_BUFFERED_INPUT_KEYS] = { 0, };
i32 kb_buf_index = 0;
b8 keyready = 0;

///////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////

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
