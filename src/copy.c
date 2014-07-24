/***********************************************************************
      LIBRARY: IMAGE - Image Manipulation and Processing Routines

      FILE:    COPY.C

      AUTHORS: Michael Garris
               Stan Janet
      DATE:    12/13/1990

      Contains routines responsible for copying a specified subimage
      of a binary image bitmap.

      ROUTINES:
#cat: copy_logical_image - uses a logical operator to copy a binary subimage
#cat:                      into a destination image at a specified location.
#cat: copy_image - copyies a binary subimage into a destination image at a
#cat:              specified location.

***********************************************************************/

/* LINTLIBRARY */

/************************************************************/
/*         Modified 12/13/90 by Stan Janet to call          */
/*                    binary_subimage_{copy,or}().          */
/*         Modified 12/21/90 by Stan Janet to call          */
/*                    binary_subimage_{and,xor}().          */
/************************************************************/

#include <stdio.h>
#include <memory.h>
#include <masks.h>
#include <bits.h>
#include <copy.h>
#include <ffpis/util/util.h>

/************************************************************/
/*         Routine:   Copy_Logical_Image()                  */
/*         Author:    Michael D. Garris                     */
/************************************************************/
/* Copy_logical_image() takes a source image region and     */
/* logically copies it to a destination image region.       */
/* NOTE: x-coords and widths are in bit units.              */
/************************************************************/

void copy_logical_image(
		int logop,
		unsigned char *frdata, int frx, int fry, int frw,int frh,
		unsigned char *todata, int tox, int toy, int tow,int toh,
		int cpw, int cph
		)
{

switch (logop) {
  case LOG_COPY:
	binary_subimage_copy(	frdata,frw,frh,
				todata,tow,toh,
				frx,fry,cpw,cph,tox,toy);
	break;
/* FIXME I can't find these functions */
#if 0
  case LOG_OR:
/* FIXME I can't find these functions */
	binary_subimage_or(	frdata,frw,frh,
				todata,tow,toh,
				frx,fry,cpw,cph,tox,toy);
	break;
  case LOG_AND:
/* FIXME I can't find these functions */
	binary_subimage_and(	frdata,frw,frh,
				todata,tow,toh,
				frx,fry,cpw,cph,tox,toy);
	break;
  case LOG_XOR:
/* FIXME I can't find these functions */
	binary_subimage_xor(	frdata,frw,frh,
				todata,tow,toh,
				frx,fry,cpw,cph,tox,toy);
	break;
#endif
  default:
	fatalerr("copy_logical_image","bad operator",(char *)NULL);
	break;
}
}

/************************************************************/
/*         Routine:   Copy_Image()                          */
/*         Author:    Michael D. Garris                     */
/*         Date:      4/30/90                               */
/************************************************************/
/* Copy_image() takes a source image region and copies it to*/
/* a destination image region.                              */
/* NOTE: x-coords and widths are in bit units.              */
/************************************************************/

void copy_image(
		unsigned char *frdata, int frx, int fry, int frw,int frh,
		unsigned char *todata, int tox, int toy, int tow,int toh,
		int cpw, int cph
		)
{

binary_subimage_copy(	frdata,frw,frh,
			todata,tow,toh,
			frx,fry,cpw,cph,tox,toy);
}
