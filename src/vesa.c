#include "include/system.h"

typedef struct VbeInfoBlock {
   char VbeSignature[4];             // == "VESA"
   u16 VbeVersion;                 // == 0x0300 for VBE 3.0
   u16 OemStringPtr[2];            // isa vbeFarPtr
   u16 Capabilities[4];
   u16 VideoModePtr[2];         // isa vbeFarPtr
   u16 TotalMemory;             // as # of 64KB blocks
} __attribute__((packed)) VbeInfoBlock;
 
VbeInfoBlock *vib = dos_alloc(512);
v86_bios(0x10, {ax:0x4f00, es:SEG(vib), di:OFF(vib)}, &out);

typedef struct {
  uint16_t attributes;
  uint8_t winA,winB;
  uint16_t granularity;
  uint16_t winsize;
  uint16_t segmentA, segmentB;
  
  VBE_FAR(realFctPtr);

  uint16_t pitch; // bytes per scanline
 
  uint16_t Xres, Yres;
  uint8_t Wchar, Ychar, planes, bpp, banks;
  uint8_t memory_model, bank_size, image_pages;
  uint8_t reserved0;
 
  uint8_t red_mask, red_position;
  uint8_t green_mask, green_position;
  uint8_t blue_mask, blue_position;
  uint8_t rsv_mask, rsv_position;
  uint8_t directcolor_attributes;
 
  uint32_t physbase;  // your LFB (Linear Framebuffer) address ;)
  uint32_t reserved1;
  uint16_t reserved2;
} __attribute__((packed)) ModeInfoBlock;


// CPP ==============================================================

// INT 0x10, AX=0x4F00: Get Controller Info. This is the one that returns the array of all supported video modes.
// INT 0x10, AX=0x4F01: Get Mode Info. Call this for each member of the mode array to find out the details of that mode.
// INT 0x10, AX=0x4F02: Set Video Mode. Call this with the mode number you decide to use.

// FIND Mode
u16 findMode(int x, int y, int d)
{
  struct VbeInfoBlock *ctrl = (VbeInfoBlock *)0x2000;
  struct ModeInfoBlock *inf = (ModeInfoBlock *)0x3000;
  u16 *modes;
  int i;
  u16 best = 0x13;
  int pixdiff, bestpixdiff = DIFF(320 * 200, x * y);
  int depthdiff, bestdepthdiff = 8 >= d ? 8 - d : (d - 8) * 2;
 
  strncpy(ctrl->VbeSignature, "VBE2", 4);
  intV86(0x10, "ax,es:di", 0x4F00, 0, ctrl); // Get Controller Info
  if ( (u16)v86.tss.eax != 0x004F ) return best;
 
  modes = (u16*)REALPTR(ctrl->VideoModePtr);
  
  for ( i = 0 ; modes[i] != 0xFFFF ; ++i ) {
      intV86(0x10, "ax,cx,es:di", 0x4F01, modes[i], 0, inf); // Get Mode Info
 
      if ( (u16)v86.tss.eax != 0x004F ) continue;
 
      // Check if this is a graphics mode with linear frame buffer support
      if ( (inf->attributes & 0x90) != 0x90 ) continue;
 
      // Check if this is a packed pixel or direct color mode
      if ( inf->memory_model != 4 && inf->memory_model != 6 ) continue;
 
      // Check if this is exactly the mode we're looking for
      if ( x == inf->XResolution && y == inf->YResolution &&
          d == inf->BitsPerPixel ) return modes[i];
 
      // Otherwise, compare to the closest match so far, remember if best
      pixdiff = DIFF(inf->Xres * inf->Yres, x * y);
      depthdiff = (inf->bpp >= d)? inf->bpp - d : (d - inf->bpp) * 2;
      if ( bestpixdiff > pixdiff ||
          (bestpixdiff == pixdiff && bestdepthdiff > depthdiff) ) {
        best = modes[i];
        bestpixdiff = pixdiff;
        bestdepthdiff = depthdiff;
      }
  }
  if ( x == 640 && y == 480 && d == 1 ) return 0x11;
  return best;
}