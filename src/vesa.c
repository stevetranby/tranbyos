#include <system.h>

typedef struct PACKED
{
    u8  VbeSignature[4];             // == "VESA"
    u16 VbeVersion;                 // == 0x0300 for VBE 3.0
    u16 OemStringPtr[2];            // isa vbeFarPtr
    u16 Capabilities[4];
    u16 VideoModePtr[2];         // isa vbeFarPtr
    u16 TotalMemory;             // as # of 64KB blocks
} VbeInfoBlock;

typedef PACKED struct {
    u16 attributes;
    u8 winA,winB;
    u16 granularity;
    u16 winsize;
    u16 segmentA, segmentB;

    //TODO: VBE_FAR(realFctPtr);

    u16 pitch; // bytes per scanline

    u16 Xres, Yres;
    u8 Wchar, Ychar, planes, bpp, banks;
    u8 memory_model, bank_size, image_pages;
    u8 reserved0;

    u8 red_mask, red_position;
    u8 green_mask, green_position;
    u8 blue_mask, blue_position;
    u8 rsv_mask, rsv_position;
    u8 directcolor_attributes;

    // your LFB (Linear Framebuffer) address ;)
    u32 physbase;
    u32 reserved1;
    u16 reserved2;
} ModeInfoBlock;

//VbeInfoBlock *vib = dos_alloc(512);
//v86_bios(0x10, {ax:0x4f00, es:SEG(vib), di:OFF(vib)}, &out);


//
//// CPP ==============================================================
//
//// INT 0x10, AX=0x4F00: Get Controller Info. This is the one that returns the array of all supported video modes.
//// INT 0x10, AX=0x4F01: Get Mode Info. Call this for each member of the mode array to find out the details of that mode.
//// INT 0x10, AX=0x4F02: Set Video Mode. Call this with the mode number you decide to use.
//
//// FIND Mode
//u16 findMode(int x, int y, int d)
//{
//  struct VbeInfoBlock *ctrl = (VbeInfoBlock *)0x2000;
//  struct ModeInfoBlock *inf = (ModeInfoBlock *)0x3000;
//  u16 *modes;
//  int i;
//  u16 best = 0x13;
//  int pixdiff, bestpixdiff = DIFF(320 * 200, x * y);
//  int depthdiff, bestdepthdiff = 8 >= d ? 8 - d : (d - 8) * 2;
//
//  strncpy(ctrl->VbeSignature, "VBE2", 4);
//  intV86(0x10, "ax,es:di", 0x4F00, 0, ctrl); // Get Controller Info
//  if ( (u16)v86.tss.eax != 0x004F ) return best;
//
//  modes = (u16*)REALPTR(ctrl->VideoModePtr);
//
//  for ( i = 0 ; modes[i] != 0xFFFF ; ++i ) {
//      intV86(0x10, "ax,cx,es:di", 0x4F01, modes[i], 0, inf); // Get Mode Info
//
//      if ( (u16)v86.tss.eax != 0x004F ) continue;
//
//      // Check if this is a graphics mode with linear frame buffer support
//      if ( (inf->attributes & 0x90) != 0x90 ) continue;
//
//      // Check if this is a packed pixel or direct color mode
//      if ( inf->memory_model != 4 && inf->memory_model != 6 ) continue;
//
//      // Check if this is exactly the mode we're looking for
//      if ( x == inf->XResolution && y == inf->YResolution &&
//          d == inf->BitsPerPixel ) return modes[i];
//
//      // Otherwise, compare to the closest match so far, remember if best
//      pixdiff = DIFF(inf->Xres * inf->Yres, x * y);
//      depthdiff = (inf->bpp >= d)? inf->bpp - d : (d - inf->bpp) * 2;
//      if ( bestpixdiff > pixdiff ||
//          (bestpixdiff == pixdiff && bestdepthdiff > depthdiff) ) {
//        best = modes[i];
//        bestpixdiff = pixdiff;
//        bestdepthdiff = depthdiff;
//      }
//  }
//  if ( x == 640 && y == 480 && d == 1 ) return 0x11;
//  return best;
//}
























unsigned char *pixel = vram + y*pitch + x*pixelwidth;
*pixel = 4;

/* only valid for 800x600x16M */
static void putpixel(unsigned char* screen, int x,int y, int color) {
    unsigned where = x*3 + y*2400;
    screen[where] = color & 255;              // BLUE
    screen[where + 1] = (color >> 8) & 255;   // GREEN
    screen[where + 2] = (color >> 16) & 255;  // RED
}
 
/* only valid for 800x600x32bpp */
static void putpixel(unsigned char* screen, int x,int y, int color) {
    unsigned where = x*4 + y*3200;
    screen[where] = color & 255;              // BLUE
    screen[where + 1] = (color >> 8) & 255;   // GREEN
    screen[where + 2] = (color >> 16) & 255;  // RED
}

static void _fillrect(unsigned char *vram, unsigned char r, unsigned char g, unsigned   char b, unsigned char w, unsigned char h) {
    unsigned char *where = vram;
    int i, j;
 
    for (i = 0; i < w; i++) {
        for (j = 0; j < h; j++) {
            //putpixel(vram, 64 + j, 64 + i, (r << 16) + (g << 8) + b);
            where[j*4] = r;
            where[j*4 + 1] = g;
            where[j*4 + 2] = b;
        }
        where+=3200;
    }
}


// holding what you need for every character of the set
font_char* font_data[CHARS];
 
// rendering one of the character, given its font_data
draw_char(screen, where, font_char*);
 
draw_string(screen, where, char* input) {
    while(*input) {
        draw_char(screen,where,font_data[input]);
        where += char_width;
        input++;
    }
}
 
draw_char(screen, where, font_char*) {
    for (l = 0; l < 8; l++) {
        for (i = 8; i > 0; i--) {
            j++;
            if ((font_char[l] & (1 << i))) {
                c = c1;
                put_pixel(j, h, c);
            }
        }
        h++;
        j = x;
    }
}

uint32_t font_data_lookup_table[16] = {
    0x00000000,
    0x000000FF,
    0x0000FF00,
    0x0000FFFF,
    0x00FF0000,
    0x00FF00FF,
    0x00FFFF00,
    0x00FFFFFF,
    0xFF000000,
    0xFF0000FF,
    0xFF00FF00,
    0xFF00FFFF,
    0xFFFF0000,
    0xFFFF00FF,
    0xFFFFFF00,
    0xFFFFFFFF
}
 
draw_char(uint8_t *where, uint32_t character, uint8_t foreground_colour, uint8_t background_colour) {
    int row;
    uint8_t row_data;
    uint32_t mask1, mask2;
    uint8_t *font_data_for_char = &system_font_data_address[character * 8];
    uint32_t packed_foreground = (foreground << 24) | (foreground << 16) | (foreground << 8) | foreground;
    uint32_t packed_background = (background << 24) | (background << 16) | (background << 8) | background;
 
    for (row = 0; row < 8; row++) {
        row_data = font_data_for_char[row];
        mask1 = font_data_lookup_table[row_data >> 16];
        mask2 = font_data_lookup_table[row_data & 0x0F];
        *(uint32_t *)where = (packed_foreground & mask1) | (packed_background & ~mask1);
        *(uint32_t *)(&where[4]) = (packed_foreground & mask2) | (packed_background & ~mask2);
        where += bytes_per_line;
    }
}