#ifndef __KB_H
#define __KB_H

// maybe move to it's own include file
// http://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html#ss1.1
/*
#define SCAN_NULL
#define SCAN_
#define SCAN_1 3
#define SCAN_2
#define SCAN_3
#define SCAN_4
#define SCAN_5
#define SCAN_6
#define SCAN_7
#define SCAN_8
#define SCAN_9
#define SCAN_0
#define SCAN_MINUS
#define SCAN_EQUAL
#define SCAN_BACKSPACE
#define SCAN_TAB
#define SCAN_Q
#define SCAN_W
#define SCAN_E
#define SCAN_
#define SCAN_
#define SCAN_
#define SCAN_
#define SCAN_
#define SCAN_
#define SCAN_
#define SCAN_
#define SCAN_
#define SCAN_
#define SCAN_
#define SCAN_
#define SCAN_
#define SCAN_
#define SCAN_
#define SCAN_
#define SCAN_
*/

#define KBDUS_DOWNARROW		137
#define KBDUS_PAGEDOWN		138
#define KBDUS_INSERT			139
#define KBDUS_CONTROL		140
#define KBDUS_LEFTSHIFT		141
#define KBDUS_RIGHTSHIFT	142
#define KBDUS_ALT				143
#define KBDUS_CAPSLOCK		144
#define KBDUS_F1				145
#define KBDUS_F2				146
#define KBDUS_F3				147
#define KBDUS_F4				148
#define KBDUS_F5				149
#define KBDUS_F6				150
#define KBDUS_F7				151
#define KBDUS_F8				152
#define KBDUS_F9				153
#define KBDUS_F10				154
#define KBDUS_F11				155
#define KBDUS_F12				156
#define KBDUS_NUMLOCK		157
#define KBDUS_SCROLLLOCK	158
#define KBDUS_HOME			159
#define KBDUS_DELETE			160
#define KBDUS_PAGEUP			161
#define KBDUS_UPARROW		162
#define KBDUS_RIGHTARROW	163
#define KBDUS_LEFTARROW		164
#define KBDUS_END				165

/* KBDUS means US Keyboard Layout. This is a scancode table
*  used to layout a standard US keyboard. I have left some
*  comments in to give you an idea of what key is what, even
*  though I set it's array index to 0. You can change that to
*  whatever you want using a macro, if you wish! */
unsigned char kbdus[128] =
{
	0,  27, '1', '2', '3', '4', '5', '6', '7', '8',			/* 9 */
	'9', '0', '-', '=', '\b', 										/* Backspace */
	'\t', 																/* Tab */
	'q', 'w', 'e', 'r', 												/* 19 */
	't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 			/* Enter key */
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

#endif
