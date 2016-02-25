#pragma once

#include "system.h"

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
#define KBDUS_INSERT		139
#define KBDUS_CONTROL		140
#define KBDUS_LEFTSHIFT		141
#define KBDUS_RIGHTSHIFT	142
#define KBDUS_ALT			143
#define KBDUS_CAPSLOCK		144
#define KBDUS_F1			145
#define KBDUS_F2			146
#define KBDUS_F3			147
#define KBDUS_F4			148
#define KBDUS_F5			149
#define KBDUS_F6			150
#define KBDUS_F7			151
#define KBDUS_F8			152
#define KBDUS_F9			153
#define KBDUS_F10			154
#define KBDUS_F11			155
#define KBDUS_F12			156
#define KBDUS_NUMLOCK		157
#define KBDUS_SCROLLLOCK	158
#define KBDUS_HOME			159
#define KBDUS_DELETE		160
#define KBDUS_PAGEUP		161
#define KBDUS_UPARROW		162
#define KBDUS_RIGHTARROW	163
#define KBDUS_LEFTARROW		164
#define KBDUS_END			165

#define KBDUS_SPACE			057

/* KBDUS means US Keyboard Layout. This is a scancode table
*  used to layout a standard US keyboard. I have left some
*  comments in to give you an idea of what key is what, even
*  though I set it's array index to 0. You can change that to
*  whatever you want using a macro, if you wish! */
extern u8 kbdus[128];
