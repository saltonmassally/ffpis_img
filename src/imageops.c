/***********************************************************************
      LIBRARY: IMAGE - Image Manipulation and Processing Routines

      FILE:    IMAGEOPS.C

      AUTHORS: Michael Garris
      DATE:    03/07/1990

      Contains routines responsible for general image operations.

      ROUTINES:
#cat: WordAlignImage - takes a binary image and pads out its scanlines to
#cat:                  an even word (16-bit) boundary.

***********************************************************************/
#include <stdio.h>
#include <string.h>
#include <defs.h>
#include "imgutil.h"
#include "memalloc.h"

/************************************************************/
/*         Routine:   WordAlignImage()                      */
/*         Author:    Michael D. Garris                     */
/*         Date:      03/07/90                              */
/************************************************************/
/* WordAlignImage() takes an input buffer and word aligns   */
/* the scan lines returning the new scan line pixel width   */
/* and the new byte length of the aligned image.            */
/************************************************************/
int WordAlignImage(
unsigned char **adata,
int *awidth,int *alength,
unsigned char  *data,
int width,int height,int depth)
{
   int i;
   int bytes_in_line, aligned_bytes_in_line, aligned_filesize;
   int aligned_pixels_in_line;
   unsigned char *inlinep, *outline;
   float pix_per_byte;

   bytes_in_line = SizeFromDepth(width,1,depth);
   aligned_pixels_in_line = WordAlignFromDepth(width,depth);
   pix_per_byte = PixPerByte(depth);
   aligned_bytes_in_line = (int)(aligned_pixels_in_line / pix_per_byte);
   if(bytes_in_line == aligned_bytes_in_line)
      return(FALSE);
   aligned_filesize = aligned_bytes_in_line * height;
   malloc_uchar(adata, aligned_filesize, "WordAlignImage : adata");
   memset((*adata), 0, aligned_filesize);
   inlinep = data;
   outline = (*adata);
   for(i = 0; i < height; i++){
      memcpy(outline,inlinep,bytes_in_line);
      outline += aligned_bytes_in_line;
      inlinep += bytes_in_line;
   }
   *awidth = aligned_pixels_in_line;
   *alength = aligned_filesize;
   return(TRUE);
}
