#include <system.h>
#include "kb.h"

// currently allow up to 255 characters to be buffered for use
char chbuf[255];
b8 keyready = 0;

///////////////////////////////////////////////////////////////
// Scan->Print

u8 kbdus[128] =
{
	0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	  /* 9 */
	'9', '0', '-', '=', '\b', 										    /* Backspace */
	'\t', 																            /* Tab */
	'q', 'w', 'e', 'r', 												      /* 19 */
	't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',     /* Enter key */
	KBDUS_CONTROL, 														/* 29 - Control */
	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', 		/* 39 */
	'\'', '`', KBDUS_LEFTSHIFT, 									/* Left shift */
	'\\', 'z', 'x', 'c', 'v', 'b', 'n', 						/* 49 */
	'm', ',', '.', '/',   KBDUS_RIGHTSHIFT,						/* Right shift */
	'*', KBDUS_ALT, 													/* Alt */
	' ',																	/* Space bar */
	KBDUS_CAPSLOCK, 													/* Caps lock */
	KBDUS_F1, KBDUS_F2, KBDUS_F3, KBDUS_F4, KBDUS_F5,
	KBDUS_F6, KBDUS_F7, KBDUS_F8, KBDUS_F9, KBDUS_F10,  	/* < ... F10 */
	KBDUS_NUMLOCK,		/* 69 - Num lock*/
	KBDUS_SCROLLLOCK,	/* Scroll Lock */
	KBDUS_HOME,				/* Home key */
	KBDUS_UPARROW,		/* Up Arrow */
	KBDUS_PAGEUP,			/* Page Up */
	'-',
	KBDUS_LEFTARROW,		/* Left Arrow */
	0,
	KBDUS_RIGHTARROW,		/* Right Arrow */
	'+',
	KBDUS_END,				/* 79 - End key*/
	KBDUS_DOWNARROW,		/* Down Arrow */
	KBDUS_PAGEDOWN,		/* Page Down */
	KBDUS_INSERT,			/* Insert Key */
	KBDUS_DELETE,			/* Delete Key */
	0,   0,   0,
	KBDUS_F11, KBDUS_F12,
	0,		/* All other keys are undefined */
};

////////////////////////////////////////////////////////

/* Handles the keyboard interrupt */
void keyboard_handler(isr_stack_state *r)
{
  unsigned char scancode;

  /* Read from the keyboard's data buffer */
  scancode = inb(0x60);

  /* If the top bit of the byte we read from the keyboard is
   *  set, that means that a key has just been released */
  if (scancode & 0x80)
  {
     /* You can use this one to see if the user released the
     *  shift, alt, or control keys... */
  }
  else
  {
		/* Here, a key was just pressed. Please note that if you
		*  hold a key down, you will get repeated key press
		*  interrupts. */

		/* Just to show you how this works, we simply translate
		*  the keyboard scancode into an ASCII value, and then
		*  display it to the screen. You can get creative and
		*  use some flags to see if a shift is pressed and use a
		*  different layout, or you can add another 128 entries
		*  to the above layout to correspond to 'shift' being
		*  held. If shift is held using the larger lookup table,
		*  you would add 128 to the scancode when you look for it */
    if(kbdus[scancode] == KBDUS_SPACE) {
			puts("Elasped Time (in seconds): ");
			printInt( timer_seconds() );
			putch('\n');
		} else {
			putch(kbdus[scancode]);
    }
  }
}

/* Installs the keyboard handler into IRQ1 */
void keyboard_install()
{
    irq_install_handler(1, keyboard_handler);
}
