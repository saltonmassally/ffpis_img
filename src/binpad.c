/***********************************************************************
      LIBRARY: IMAGE - Image Manipulation and Processing Routines

      FILE:    BINPAD.C

      AUTHORS: Stan Janet
      DATE:    12/14/1991

      Contains routines responsible for enlarging and padding a
      binary image bitmap.

      ROUTINES:
#cat: binary_image_pad - enlarges the dimensions of a binary bitmap by
#cat:                    padding its byte-aligned width and pixel height.
#cat: binary_image_mpad - enlarges the dimensions of a binary bitmap by
#cat:                     padding accordiing to specified multiples.

***********************************************************************/

/* LINTLIBRARY */

#include <stdio.h>
#include <stdlib.h>
#include <ffpis/util/util.h>
#include "copy.h"
#include <limits.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/param.h>

#define SUBR_STR		"binary_image_pad"
#ifndef roundup
#define roundup(x, y) ((((x)+((y)-1))/(y))*(y))
#endif
#ifndef BITSPERBYTE
#define BITSPERBYTE CHAR_BIT
#endif


int binary_image_pad( u_char **image,
u_int w,u_int  h,u_int  padw,u_int  padh, int bg)
{
u_char *new_image;
u_int new_size;

if (image == (u_char **) NULL)
	fatalerr(SUBR_STR,"null image pointer address",(char *)NULL);
if (*image == (u_char *) NULL)
	fatalerr(SUBR_STR,"null image pointer",(char *)NULL);

if (padw == 0)
	fatalerr(SUBR_STR,"pad width is zero",(char *)NULL);
if (padh == 0)
	fatalerr(SUBR_STR,"pad height is zero",(char *)NULL);
if (padw % BITSPERBYTE)
	fatalerr(SUBR_STR,"pad width is not a multiple of 8",(char *)NULL);

if ((w == 0) || (h == 0))		/* no image to pad */
	return 0;

if ((w == padw) && (h == padh))		/* no growth */
	return 0;

new_size = (padw / BITSPERBYTE) * padh;

new_image = (u_char *) malloc(new_size);
if (new_image == (u_char *) NULL)
	fatalerr(SUBR_STR,"malloc failed",(char *)NULL);

(void) memset((char *)new_image, (bg ? ~0 : 0), (int)new_size);
binary_subimage_copy(*image,w,h,new_image,padw,padh,0,0,w,h,0,0);

*image = new_image;
return 1;
}




#undef SUBR_STR
#define SUBR_STR	"binary_image_mpad"


int binary_image_mpad( u_char **image,
u_int *w,u_int  *h,u_int  mpadw,u_int  mpadh,u_int bg)
{
u_int new_w, new_h;
int n;

if (image == (u_char **) NULL)
	fatalerr(SUBR_STR,"null image pointer address",(char *)NULL);
if (*image == (u_char *) NULL)
	fatalerr(SUBR_STR,"null image pointer",(char *)NULL);
if (w == (u_int *) NULL)
	fatalerr(SUBR_STR,"null width pointer",(char *)NULL);
if (h == (u_int *) NULL)
	fatalerr(SUBR_STR,"null height pointer",(char *)NULL);

if (mpadw == 0)
	fatalerr(SUBR_STR,"pad width is zero",(char *)NULL);
if (mpadh == 0)
	fatalerr(SUBR_STR,"pad height is zero",(char *)NULL);
if (mpadw % BITSPERBYTE)
	fatalerr(SUBR_STR,"pad width is not a multiple of 8",(char *)NULL);

if ((*w == 0) || (*h == 0))
	return 0;

if ((*w % mpadw == 0) && (*h % mpadh == 0))
	return 0;

new_w = roundup(*w,mpadw);
new_h = roundup(*h,mpadh);
n = binary_image_pad(image,*w,*h,new_w,new_h,bg);
if (n) {
	*w = new_w;
	*h = new_h;
}
return n;
}
