/***********************************************************************
      LIBRARY: JPEGB - Baseline (Lossy) JPEG Utilities

      FILE:    PPI.C
      AUTHORS: Michael Garris
               Craig Watson
      DATE:    01/09/2001

      Contains routines responsible for determining a compressed image's
      scan resolution in units of pixels per inch.

      ROUTINES:
#cat: get_ppi_jpegb - If possible, computes and returns the scan resolution
#cat:                 in pixels per inch of a JPEGB datastream given a
#cat:                 JPEGB decompess structure.

***********************************************************************/

#include <stdio.h>
#include <jpegb.h>

#define CM_PER_INCH   2.54

/************************************************************************/
int get_ppi_jpegb(int *oppi, j_decompress_ptr cinfo)
{
   int ppi;

   /* Get and set scan density in pixels per inch. */
   switch(cinfo->density_unit){
      /* pixels per inch */
      case 1:
         /* take the horizontal pixel density, even if the vertical is */
         /* not the same */
         ppi = cinfo->X_density;
         break;
      /* pixels per cm */
      case 2:
         /* compute ppi from horizontal density even if not */
         /* equal to vertical */
         ppi = (int)((cinfo->X_density * CM_PER_INCH) + 0.5);
         break;
      /* unknown density */
      case 0:
         /* set ppi to -1 == UNKNOWN */
         ppi = -1;
         break;
      /* ERROR */
      default:
         fprintf(stderr, "ERROR : get_ppi_jpegb : ");
         fprintf(stderr, "illegal density unit = %d\n", cinfo->density_unit);
         return(-2);
   }

   *oppi = ppi;

   return(0);
}
