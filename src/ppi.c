/***********************************************************************
      LIBRARY: JPEGL - Lossless JPEG Image Compression

      FILE:    PPI.C
      AUTHORS: Craig Watson
               Michael Garris
      DATE:    01/17/2001

      Contains routines responsible for determining the scan resolution
      in units of pixels per inch from a JPEGL compressed datastream.

      ROUTINES:
#cat: get_ppi_jpegl - Given a JFIF Header from a JPEGL compressed
#cat:                 datastream, extracts/derives the pixel scan
#cat:                 resolution in units of pixel per inch.

***********************************************************************/

#include <stdio.h>
#include <jpegl.h>

#define CM_PER_INCH   2.54

/************************************************************************/
int get_ppi_jpegl(int *oppi, JFIF_HEADER *jfif_header)
{
   int ppi;

   /* Get and set scan density in pixels per inch. */
   switch(jfif_header->units){
      /* pixels per inch */
      case 1:
         /* take the horizontal pixel density, even if the vertical is */
         /* not the same */
         ppi = jfif_header->dx;
         break;
      /* pixels per cm */
      case 2:
         /* compute ppi from horizontal density even if not */
         /* equal to vertical */
         ppi = (int)((jfif_header->dx * CM_PER_INCH) + 0.5);
         break;
      /* unknown density */
      case 0:
         /* set ppi to -1 == UNKNOWN */
         ppi = -1;
         break;
      /* ERROR */
      default:
         fprintf(stderr, "ERROR : get_ppi_jpegl : ");
         fprintf(stderr, "illegal density unit = %d\n", jfif_header->units);
         return(-2);
   }

   *oppi = ppi;

   return(0);
}
