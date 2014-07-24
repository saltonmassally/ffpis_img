/***********************************************************************
      LIBRARY: IMAGE - Image Manipulation and Processing Routines

      FILE:    BITMASKS.C

      AUTHOR:  Stan Janet
      DATE:    12/14/1991

      Contains global bit mask declarations used by routines responsible
      for manipulating a binary image bitmap.

***********************************************************************/
#include <limits.h>
#include <sys/types.h>

#ifndef BITSPERBYTE
#define BITSPERBYTE CHAR_BIT
#endif

u_char mask_begin_1[BITSPERBYTE] = {
	0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE
};

u_char mask_begin_0[BITSPERBYTE] = {
	0x00, 0x7F, 0x3F, 0x1F, 0x0F, 0x07, 0x03, 0x01
};

u_char mask_end_0[BITSPERBYTE] = {
	0x00, 0xFE, 0xFC, 0xF8, 0xF0, 0xE0, 0xC0, 0x80
};

u_char mask_end_1[BITSPERBYTE] = {
	0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F
};


u_char mask_1_at[BITSPERBYTE] = {
	0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
};

u_char mask_0_at[BITSPERBYTE] = {
	0x7F, 0xBF, 0xDF, 0xEF, 0xF7, 0xFB, 0xFD, 0xFE
};
