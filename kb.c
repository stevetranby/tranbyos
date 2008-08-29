#include <system.h>
#include <kb.h>

/* Handles the keyboard interrupt */
void keyboard_handler(struct regs *r)
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
        if(kbdus[scancode] == KBDUS_INSERT) {
			puts("Elasped Time (in seconds): ");
			printInt( time_s() );
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
