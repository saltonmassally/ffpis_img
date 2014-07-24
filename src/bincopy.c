/***********************************************************************
      LIBRARY: IMAGE - Image Manipulation and Processing Routines

      FILE:    BINCOPY.C

      AUTHORS: Stan Janet
      DATE:    11/16/1990

      Contains routines responsible for copying a specified subimage
      of a binary image bitmap.

      ROUTINES:
#cat: binary_subimage_copy_8 - copies a binary subimage that is a multiple
#cat:                          of 8 bits wide.
#cat: binary_subimage_copy_gt - special condition when copying a
#cat:                           binary subimage.
#cat: binary_subimage_copy_lt - special condition when copying a
#cat:                           binary subimage.
#cat: binary_subimage_copy_eq - special condition when copying a
#cat:                           binary subimage.
#cat: binary_subimage_copy - copies a binary subimage to another image
#cat:                        at a specified location.

***********************************************************************/

/* LINTLIBRARY */

#include <stdio.h>
#include <memory.h>
#include <ffpis/util/util.h>
#include <limits.h>
#include <sys/types.h>
#include <masks.h>
#include <bitmasks.h>
#include <bits.h>
#include <copy.h>
#include <binops.h>

#ifndef BITSPERBYTE
#define BITSPERBYTE CHAR_BIT
#endif


void binary_subimage_copy_8 (
	u_char *src, int  srcw,int srch,
	u_char  *dst, int dstw, int dsth,
       int srcx,int srcy,
       int  cpw,int cph,
       int dstx,int dsty)
{
int i, j, i_inv, j_inv, cpbw, srcbw, dstbw, dx, dy;
u_char m0, m1, m2;
register int bytes;
register u_char c;



if (src == (u_char *) NULL)
	fatalerr("binary_subimage_copy_8","Null source image pointer",(char *)NULL);

if (dst == (u_char *) NULL)
	fatalerr("binary_subimage_copy_8","Null destination image pointer",(char *)NULL);

if ((srcw < 0) || (srch < 0))
	fatalerr("binary_subimage_copy_8","Negative source image dimension(s)",(char *)NULL);

if ((dstw < 0) || (dsth < 0))
	fatalerr("binary_subimage_copy_8","Negative destination image dimension(s)",(char *)NULL);

if ((cpw < 0) || (cph < 0))
	fatalerr("binary_subimage_copy_8","Negative subimage dimension(s)",(char *)NULL);

if (srcw % BITSPERBYTE)
	fatalerr("binary_subimage_copy_8",
		"Source image width must be a multiple of 8",(char *)NULL);

if (dstw % BITSPERBYTE)
	fatalerr("binary_subimage_copy_8",
		"Destination image width must be byte aligned",(char *)NULL);

if (((srcx + cpw) > srcw) || ((srcy + cph) > srch))
	fatalerr("binary_subimage_copy_8",
		"Subimage exceeds source image dimension(s)",(char *)NULL);

if (((dstx + cpw) > dstw) || ((dsty + cph) > dsth))
	fatalerr("binary_subimage_copy_8",
		"Subimage exceeds destination image dimension(s)",(char *)NULL);


if (!cpw || !cph)
	return;

if (!srcw || !srch)
	return;

if (!dstw || !dsth)
	return;

	i = srcx % BITSPERBYTE;
	i_inv = BITSPERBYTE - i;

j = dstx % BITSPERBYTE;
j_inv = BITSPERBYTE - j;

	if (cpw % BITSPERBYTE)
		fatalerr("binary_subimage_copy_8",
			"Copy width not a multiple of eight",(char *)NULL);


	srcbw = srcw / BITSPERBYTE;

dstbw = dstw / BITSPERBYTE;

cpbw  = cpw  / BITSPERBYTE;


	/* Increment src to the byte in the upper left corner of the */
	/* subimage to be copied. Increment dst to the upper left corner */
	/* of the byte in the destination that will be modified. */
	src += srcy * srcbw + srcx / BITSPERBYTE;

dst += dsty * dstbw + dstx / BITSPERBYTE;


	/* Set dx (dy) to be the increment needed to get src (dst) to the */
	/* first byte in the next scan line to be copied (or modified) from */
	/* the last byte in the current scan line. */
	dx = srcbw - cpbw;

dy = dstbw - cpbw;


if (!i && !j) {			/* Both src & dst are byte-aligned. */
	while (cph--) {
		bytes = cpbw;
		while (bytes--)
			*dst++ = *src++;


		src += dx;
		dst += dy;
	}
	return;
}






if (!j) {			/* Dst is byte-aligned. */
	while (cph--) {
		bytes = cpbw;
		while (bytes--) {
			c = (*src << i) | (*(src+1) >> i_inv);
			*dst++ = c;
			src++;
		}
		src += dx;
		dst += dy;
	}
	return;
}








if (!i) {					/* Src is byte-aligned. */
	m1 = mask_end_0[j_inv];				/* eg. 11110000 */
	m2 = mask_begin_0[j];				/* eg. 00001111 */

	while (cph--) {
		bytes = cpbw;
		while (bytes--) {
			c = *src++;
			*dst = (*dst & m1) | (c >> j);
			dst++;
			*dst = (*dst & m2) | (c << j_inv);
		}
		;
		src += dx;
		dst += dy;
	}
	return;
}






if (i == j) {
	m0 = mask_begin_1[i];
	m1 = mask_end_0[i_inv];
	m2 = mask_end_1[i_inv];

	/* Neither src nor dst is byte-aligned, but they */
	/* are offset by the same amount, so we can copy */
	/* (cpbw - 1) bytes directly from src to dst. The */
	/* bits in the other byte are on the ends of the */
	/* scan line and are treated specially. */


	if (cpbw == 1) {
		while (cph--) {
			*dst = (*dst & m0) | (*src++ & m2);
			dst++;
			*dst = (*dst & m2) | (*src & m0);
			;
			src += dx;
			dst += dy;
		}
		return;
	}

	while (cph--) {
		*dst = (*dst & m0) | (*src++ & m2);
		dst++;

		bytes = cpbw - 1;
		while (bytes--)
			*dst++ = *src++;

		*dst = (*dst & m2) | (*src & m0);
		;
		src += dx;
		dst += dy;
	}
	return;
}






m1 = mask_end_0[j_inv];		/* eg. 11111000 */
m2 = mask_begin_0[j];		/* eg. 00000111 */


while (cph--) {				/* The absolute worst case. Neither */
	bytes = cpbw;			/* src nor dst is byte-aligned, and */
	while (bytes--) {		/* they are offset by diff. amounts. */
		c = (*src << i) | (*(src+1) >> i_inv);
		*dst = (*dst & m1) | (c >> j);
		dst++;
		*dst = (*dst & m2) | (c << j_inv);
		src++;
	}
	;
	src += dx;
	dst += dy;
}
}
/* LINTLIBRARY */

/****************************************************************/
/*								*/
/*	Routines:	binary_subimage_<op>_8()		*/
/*			binary_subimage_<op>_gt()		*/
/*			binary_subimage_<op>_lt()		*/
/*			binary_subimage_<op>_eq()		*/
/*				where <op> is copy, and, or,	*/
/*				xor, invert, zero, or one	*/
/*								*/
/*	Author:		Stan Janet				*/
/*	Date:		11/16/90				*/
/*								*/
/* binary_subimage_copy_8() is a bit-level copy utility for	*/
/*	subimages that are a multiple of 8 bits wide.		*/
/* binary_subimage_copy_{gt,lt,eq}() are bit-level copy		*/
/*	utilities for subimages that are not a multiple of 8	*/
/*	bits wide and where the space remaining in the last	*/
/*	byte in every destination scan line will be (respect-	*/
/*	ively) greater than, less than, or equal to the		*/
/*	number of bits by which the subimage width exceeds 8.	*/
/* If binary_subimage_copy() is told to copy a subimage		*/
/*	that is a multiple of 8 bits wide, it calls		*/
/*	binary_subimage_copy_8(). Otherwise it calls one of	*/
/*	the other functions as appropriate.			*/
/* binary_subimage_or_8() is a bit-level logical or utility for	*/
/*	subimages that are a multiple of 8 bits wide.		*/
/* binary_subimage_{or,and,xor,invert}_{8,gt,lt,eq}() are	*/
/*	bit-level logical or, and, xor and invert utilities for */
/*	subimages that are analogous to	the copy utilities	*/
/*	described above.					*/
/* binary_subimage_{zero,one}_{8,gt,lt,eq} are bit-level util-	*/
/*	ities for setting all bits in a subimage to zero and	*/
/*	one, respectively.					*/
/* All dimensions are taken to be in bit units.			*/
/****************************************************************/


#include <stdio.h>
#include <memory.h>
#include <sys/types.h>

#include <masks.h>
#include <bitmasks.h>
#include <bits.h>
#include <copy.h>
#include <binops.h>
















void binary_subimage_copy_gt (
	u_char *src, int  srcw,int srch,
	u_char  *dst, int dstw, int dsth,
       int srcx,int srcy,
       int  cpw,int cph,
       int dstx,int dsty)
{
int i, j, i_inv, j_inv, cpbw, srcbw, dstbw, dx, dy;
u_char m0, m1, m2;
register int bytes;
register u_char c;

	u_char m3, m4;
	int b, b_inv, diff;


if (src == (u_char *) NULL)
	fatalerr("binary_subimage_copy_gt","Null source image pointer",(char *)NULL);

if (dst == (u_char *) NULL)
	fatalerr("binary_subimage_copy_gt","Null destination image pointer",(char *)NULL);

if ((srcw < 0) || (srch < 0))
	fatalerr("binary_subimage_copy_gt","Negative source image dimension(s)",(char *)NULL);

if ((dstw < 0) || (dsth < 0))
	fatalerr("binary_subimage_copy_gt","Negative destination image dimension(s)",(char *)NULL);

if ((cpw < 0) || (cph < 0))
	fatalerr("binary_subimage_copy_gt","Negative subimage dimension(s)",(char *)NULL);

if (srcw % BITSPERBYTE)
	fatalerr("binary_subimage_copy_gt",
		"Source image width must be a multiple of 8",(char *)NULL);

if (dstw % BITSPERBYTE)
	fatalerr("binary_subimage_copy_gt",
		"Destination image width must be byte aligned",(char *)NULL);

if (((srcx + cpw) > srcw) || ((srcy + cph) > srch))
	fatalerr("binary_subimage_copy_gt",
		"Subimage exceeds source image dimension(s)",(char *)NULL);

if (((dstx + cpw) > dstw) || ((dsty + cph) > dsth))
	fatalerr("binary_subimage_copy_gt",
		"Subimage exceeds destination image dimension(s)",(char *)NULL);


if (!cpw || !cph)
	return;

if (!srcw || !srch)
	return;

if (!dstw || !dsth)
	return;

	i = srcx % BITSPERBYTE;
	i_inv = BITSPERBYTE - i;

j = dstx % BITSPERBYTE;
j_inv = BITSPERBYTE - j;

		b = cpw % BITSPERBYTE;
		b_inv = BITSPERBYTE - b;
		diff = j_inv - b;
		if (diff <= 0)
		fatalerr("binary_subimage_copy_gt","bad diff",(char *)NULL);


	srcbw = srcw / BITSPERBYTE;

dstbw = dstw / BITSPERBYTE;

cpbw  = cpw  / BITSPERBYTE;


	/* Increment src to the byte in the upper left corner of the */
	/* subimage to be copied. Increment dst to the upper left corner */
	/* of the byte in the destination that will be modified. */
	src += srcy * srcbw + srcx / BITSPERBYTE;

dst += dsty * dstbw + dstx / BITSPERBYTE;


	/* Set dx (dy) to be the increment needed to get src (dst) to the */
	/* first byte in the next scan line to be copied (or modified) from */
	/* the last byte in the current scan line. */
	dx = srcbw - cpbw;

dy = dstbw - cpbw;


if (!i && !j) {			/* Both src & dst are byte-aligned. */
	m3 = mask_end_1[b_inv];	/* eg. 00000111 */
	m4 = mask_begin_1[b];	/* eg. 11111000 */
	while (cph--) {
		bytes = cpbw;
		while (bytes--)
			*dst++ = *src++;

		*dst = (*dst & m3) | (*src & m4);

		src += dx;
		dst += dy;
	}
	return;
}






if (!j) {			/* Dst is byte-aligned. */
	m3 = mask_end_1[b_inv];	/* eg. 00000111 */
	m4 = mask_begin_1[b];	/* eg. 11111000 */
	while (cph--) {
		bytes = cpbw;
		while (bytes--) {
			c = (*src << i) | (*(src+1) >> i_inv);
			*dst++ = c;
			src++;
		}
		c = (*src << i) | (*(src+1) >> i_inv);
		*dst = (*dst & m3) | (c & m4);
		src += dx;
		dst += dy;
	}
	return;
}








if (!i) {					/* Src is byte-aligned. */
	m1 = mask_end_0[j_inv];				/* eg. 11110000 */
	m2 = mask_begin_0[j];				/* eg. 00001111 */
	if (diff > 0) {
		/* j_inv > b */
		m3 = m1 | mask_end_1[diff];		/* eg. 11110011 */
		m4 = mask_end_0[diff];			/* eg. 11111100 */
	} else if (diff) {
		/* j_inv < b */
		m3 = mask_end_1[BITSPERBYTE+diff];	/* eg. 00111111 */
		m4 = mask_begin_1[-diff];		/* eg. 11000000 */
	}

	while (cph--) {
		bytes = cpbw;
		while (bytes--) {
			c = *src++;
			*dst = (*dst & m1) | (c >> j);
			dst++;
			*dst = (*dst & m2) | (c << j_inv);
		}
		binary_fill_partial(BINARY_COPY,src,i,dst,j,b);
		src += dx;
		dst += dy;
	}
	return;
}






if (i == j) {
	m0 = mask_begin_1[i];
	m1 = mask_end_0[i_inv];
	m2 = mask_end_1[i_inv];
	if (diff > 0) {
		m3 = m1 | mask_end_1[diff];
		m4 = mask_end_0[diff];
	} else if (diff) { 
		m3 = mask_end_1[BITSPERBYTE+diff];
		m4 = mask_begin_1[-diff];
	}

	/* Neither src nor dst is byte-aligned, but they */
	/* are offset by the same amount, so we can copy */
	/* (cpbw - 1) bytes directly from src to dst. The */
	/* bits in the other byte are on the ends of the */
	/* scan line and are treated specially. */

	/* We don't need this case in binary_subimage_*_8(). If cpbw==0, */
	/* then cpw was 0 and we would have already returned. */

	if (! cpbw) {
		while (cph--) {
			binary_fill_partial(BINARY_COPY,src,i,dst,j,b);
			src += dx;
			dst += dy;
		}
		return;
	}

	if (cpbw == 1) {
		while (cph--) {
			*dst = (*dst & m0) | (*src++ & m2);
			dst++;
			*dst = (*dst & m2) | (*src & m0);
			binary_fill_partial(BINARY_COPY,src,i,dst,j,b);
			src += dx;
			dst += dy;
		}
		return;
	}

	while (cph--) {
		*dst = (*dst & m0) | (*src++ & m2);
		dst++;

		bytes = cpbw - 1;
		while (bytes--)
			*dst++ = *src++;

		*dst = (*dst & m2) | (*src & m0);
		binary_fill_partial(BINARY_COPY,src,i,dst,j,b);
		src += dx;
		dst += dy;
	}
	return;
}






m1 = mask_end_0[j_inv];		/* eg. 11111000 */
m2 = mask_begin_0[j];		/* eg. 00000111 */

	if (diff > 0) {
		m3 = m1 | mask_end_1[diff];
		m4 = mask_end_0[diff];
	} else if (diff) { 
		m3 = mask_end_1[BITSPERBYTE+diff];
		m4 = mask_begin_1[-diff];
	}

while (cph--) {				/* The absolute worst case. Neither */
	bytes = cpbw;			/* src nor dst is byte-aligned, and */
	while (bytes--) {		/* they are offset by diff. amounts. */
		c = (*src << i) | (*(src+1) >> i_inv);
		*dst = (*dst & m1) | (c >> j);
		dst++;
		*dst = (*dst & m2) | (c << j_inv);
		src++;
	}
	binary_fill_partial(BINARY_COPY,src,i,dst,j,b);
	src += dx;
	dst += dy;
}
}
/* LINTLIBRARY */

/****************************************************************/
/*								*/
/*	Routines:	binary_subimage_<op>_8()		*/
/*			binary_subimage_<op>_gt()		*/
/*			binary_subimage_<op>_lt()		*/
/*			binary_subimage_<op>_eq()		*/
/*				where <op> is copy, and, or,	*/
/*				xor, invert, zero, or one	*/
/*								*/
/*	Author:		Stan Janet				*/
/*	Date:		11/16/90				*/
/*								*/
/* binary_subimage_copy_8() is a bit-level copy utility for	*/
/*	subimages that are a multiple of 8 bits wide.		*/
/* binary_subimage_copy_{gt,lt,eq}() are bit-level copy		*/
/*	utilities for subimages that are not a multiple of 8	*/
/*	bits wide and where the space remaining in the last	*/
/*	byte in every destination scan line will be (respect-	*/
/*	ively) greater than, less than, or equal to the		*/
/*	number of bits by which the subimage width exceeds 8.	*/
/* If binary_subimage_copy() is told to copy a subimage		*/
/*	that is a multiple of 8 bits wide, it calls		*/
/*	binary_subimage_copy_8(). Otherwise it calls one of	*/
/*	the other functions as appropriate.			*/
/* binary_subimage_or_8() is a bit-level logical or utility for	*/
/*	subimages that are a multiple of 8 bits wide.		*/
/* binary_subimage_{or,and,xor,invert}_{8,gt,lt,eq}() are	*/
/*	bit-level logical or, and, xor and invert utilities for */
/*	subimages that are analogous to	the copy utilities	*/
/*	described above.					*/
/* binary_subimage_{zero,one}_{8,gt,lt,eq} are bit-level util-	*/
/*	ities for setting all bits in a subimage to zero and	*/
/*	one, respectively.					*/
/* All dimensions are taken to be in bit units.			*/
/****************************************************************/


#include <stdio.h>
#include <memory.h>
#include <sys/types.h>

#include <masks.h>
#include <bitmasks.h>
#include <bits.h>
#include <copy.h>
#include <binops.h>
















void binary_subimage_copy_lt (
	u_char *src, int  srcw,int srch,
	u_char  *dst, int dstw, int dsth,
       int srcx,int srcy,
       int  cpw,int cph,
       int dstx,int dsty)
{
int i, j, i_inv, j_inv, cpbw, srcbw, dstbw, dx, dy;
u_char m0, m1, m2;
register int bytes;
register u_char c;

	u_char m3, m4;
	int b, b_inv, diff;


if (src == (u_char *) NULL)
	fatalerr("binary_subimage_copy_lt","Null source image pointer",(char *)NULL);

if (dst == (u_char *) NULL)
	fatalerr("binary_subimage_copy_lt","Null destination image pointer",(char *)NULL);

if ((srcw < 0) || (srch < 0))
	fatalerr("binary_subimage_copy_lt","Negative source image dimension(s)",(char *)NULL);

if ((dstw < 0) || (dsth < 0))
	fatalerr("binary_subimage_copy_lt","Negative destination image dimension(s)",(char *)NULL);

if ((cpw < 0) || (cph < 0))
	fatalerr("binary_subimage_copy_lt","Negative subimage dimension(s)",(char *)NULL);

if (srcw % BITSPERBYTE)
	fatalerr("binary_subimage_copy_lt",
		"Source image width must be a multiple of 8",(char *)NULL);

if (dstw % BITSPERBYTE)
	fatalerr("binary_subimage_copy_lt",
		"Destination image width must be byte aligned",(char *)NULL);

if (((srcx + cpw) > srcw) || ((srcy + cph) > srch))
	fatalerr("binary_subimage_copy_lt",
		"Subimage exceeds source image dimension(s)",(char *)NULL);

if (((dstx + cpw) > dstw) || ((dsty + cph) > dsth))
	fatalerr("binary_subimage_copy_lt",
		"Subimage exceeds destination image dimension(s)",(char *)NULL);


if (!cpw || !cph)
	return;

if (!srcw || !srch)
	return;

if (!dstw || !dsth)
	return;

	i = srcx % BITSPERBYTE;
	i_inv = BITSPERBYTE - i;

j = dstx % BITSPERBYTE;
j_inv = BITSPERBYTE - j;

		b = cpw % BITSPERBYTE;
		b_inv = BITSPERBYTE - b;
		diff = j_inv - b;
		if (diff >= 0)
		fatalerr("binary_subimage_copy_lt","bad diff",(char *)NULL);


	srcbw = srcw / BITSPERBYTE;

dstbw = dstw / BITSPERBYTE;

cpbw  = cpw  / BITSPERBYTE;


	/* Increment src to the byte in the upper left corner of the */
	/* subimage to be copied. Increment dst to the upper left corner */
	/* of the byte in the destination that will be modified. */
	src += srcy * srcbw + srcx / BITSPERBYTE;

dst += dsty * dstbw + dstx / BITSPERBYTE;


	/* Set dx (dy) to be the increment needed to get src (dst) to the */
	/* first byte in the next scan line to be copied (or modified) from */
	/* the last byte in the current scan line. */
	dx = srcbw - cpbw;

dy = dstbw - cpbw;


if (!i && !j) {			/* Both src & dst are byte-aligned. */
	m3 = mask_end_1[b_inv];	/* eg. 00000111 */
	m4 = mask_begin_1[b];	/* eg. 11111000 */
	while (cph--) {
		bytes = cpbw;
		while (bytes--)
			*dst++ = *src++;

		*dst = (*dst & m3) | (*src & m4);

		src += dx;
		dst += dy;
	}
	return;
}






if (!j) {			/* Dst is byte-aligned. */
	m3 = mask_end_1[b_inv];	/* eg. 00000111 */
	m4 = mask_begin_1[b];	/* eg. 11111000 */
	while (cph--) {
		bytes = cpbw;
		while (bytes--) {
			c = (*src << i) | (*(src+1) >> i_inv);
			*dst++ = c;
			src++;
		}
		c = (*src << i) | (*(src+1) >> i_inv);
		*dst = (*dst & m3) | (c & m4);
		src += dx;
		dst += dy;
	}
	return;
}








if (!i) {					/* Src is byte-aligned. */
	m1 = mask_end_0[j_inv];				/* eg. 11110000 */
	m2 = mask_begin_0[j];				/* eg. 00001111 */
	if (diff > 0) {
		/* j_inv > b */
		m3 = m1 | mask_end_1[diff];		/* eg. 11110011 */
		m4 = mask_end_0[diff];			/* eg. 11111100 */
	} else if (diff) {
		/* j_inv < b */
		m3 = mask_end_1[BITSPERBYTE+diff];	/* eg. 00111111 */
		m4 = mask_begin_1[-diff];		/* eg. 11000000 */
	}

	while (cph--) {
		bytes = cpbw;
		while (bytes--) {
			c = *src++;
			*dst = (*dst & m1) | (c >> j);
			dst++;
			*dst = (*dst & m2) | (c << j_inv);
		}
		binary_fill_partial(BINARY_COPY,src,i,dst,j,b);
		src += dx;
		dst += dy;
	}
	return;
}






if (i == j) {
	m0 = mask_begin_1[i];
	m1 = mask_end_0[i_inv];
	m2 = mask_end_1[i_inv];
	if (diff > 0) {
		m3 = m1 | mask_end_1[diff];
		m4 = mask_end_0[diff];
	} else if (diff) { 
		m3 = mask_end_1[BITSPERBYTE+diff];
		m4 = mask_begin_1[-diff];
	}

	/* Neither src nor dst is byte-aligned, but they */
	/* are offset by the same amount, so we can copy */
	/* (cpbw - 1) bytes directly from src to dst. The */
	/* bits in the other byte are on the ends of the */
	/* scan line and are treated specially. */

	/* We don't need this case in binary_subimage_*_8(). If cpbw==0, */
	/* then cpw was 0 and we would have already returned. */

	if (! cpbw) {
		while (cph--) {
			binary_fill_partial(BINARY_COPY,src,i,dst,j,b);
			src += dx;
			dst += dy;
		}
		return;
	}

	if (cpbw == 1) {
		while (cph--) {
			*dst = (*dst & m0) | (*src++ & m2);
			dst++;
			*dst = (*dst & m2) | (*src & m0);
			binary_fill_partial(BINARY_COPY,src,i,dst,j,b);
			src += dx;
			dst += dy;
		}
		return;
	}

	while (cph--) {
		*dst = (*dst & m0) | (*src++ & m2);
		dst++;

		bytes = cpbw - 1;
		while (bytes--)
			*dst++ = *src++;

		*dst = (*dst & m2) | (*src & m0);
		binary_fill_partial(BINARY_COPY,src,i,dst,j,b);
		src += dx;
		dst += dy;
	}
	return;
}






m1 = mask_end_0[j_inv];		/* eg. 11111000 */
m2 = mask_begin_0[j];		/* eg. 00000111 */

	if (diff > 0) {
		m3 = m1 | mask_end_1[diff];
		m4 = mask_end_0[diff];
	} else if (diff) { 
		m3 = mask_end_1[BITSPERBYTE+diff];
		m4 = mask_begin_1[-diff];
	}

while (cph--) {				/* The absolute worst case. Neither */
	bytes = cpbw;			/* src nor dst is byte-aligned, and */
	while (bytes--) {		/* they are offset by diff. amounts. */
		c = (*src << i) | (*(src+1) >> i_inv);
		*dst = (*dst & m1) | (c >> j);
		dst++;
		*dst = (*dst & m2) | (c << j_inv);
		src++;
	}
	binary_fill_partial(BINARY_COPY,src,i,dst,j,b);
	src += dx;
	dst += dy;
}
}
/* LINTLIBRARY */

/****************************************************************/
/*								*/
/*	Routines:	binary_subimage_<op>_8()		*/
/*			binary_subimage_<op>_gt()		*/
/*			binary_subimage_<op>_lt()		*/
/*			binary_subimage_<op>_eq()		*/
/*				where <op> is copy, and, or,	*/
/*				xor, invert, zero, or one	*/
/*								*/
/*	Author:		Stan Janet				*/
/*	Date:		11/16/90				*/
/*								*/
/* binary_subimage_copy_8() is a bit-level copy utility for	*/
/*	subimages that are a multiple of 8 bits wide.		*/
/* binary_subimage_copy_{gt,lt,eq}() are bit-level copy		*/
/*	utilities for subimages that are not a multiple of 8	*/
/*	bits wide and where the space remaining in the last	*/
/*	byte in every destination scan line will be (respect-	*/
/*	ively) greater than, less than, or equal to the		*/
/*	number of bits by which the subimage width exceeds 8.	*/
/* If binary_subimage_copy() is told to copy a subimage		*/
/*	that is a multiple of 8 bits wide, it calls		*/
/*	binary_subimage_copy_8(). Otherwise it calls one of	*/
/*	the other functions as appropriate.			*/
/* binary_subimage_or_8() is a bit-level logical or utility for	*/
/*	subimages that are a multiple of 8 bits wide.		*/
/* binary_subimage_{or,and,xor,invert}_{8,gt,lt,eq}() are	*/
/*	bit-level logical or, and, xor and invert utilities for */
/*	subimages that are analogous to	the copy utilities	*/
/*	described above.					*/
/* binary_subimage_{zero,one}_{8,gt,lt,eq} are bit-level util-	*/
/*	ities for setting all bits in a subimage to zero and	*/
/*	one, respectively.					*/
/* All dimensions are taken to be in bit units.			*/
/****************************************************************/


#include <stdio.h>
#include <memory.h>
#include <sys/types.h>

#include <masks.h>
#include <bitmasks.h>
#include <bits.h>
#include <copy.h>
#include <binops.h>
















void binary_subimage_copy_eq (
	u_char *src, int  srcw,int srch,
	u_char  *dst, int dstw, int dsth,
       int srcx,int srcy,
       int  cpw,int cph,
       int dstx,int dsty)
{
int i, j, i_inv, j_inv, cpbw, srcbw, dstbw, dx, dy;
u_char m0, m1, m2;
register int bytes;
register u_char c;

	u_char m3, m4;
	int b, b_inv, diff;


if (src == (u_char *) NULL)
	fatalerr("binary_subimage_copy_eq","Null source image pointer",(char *)NULL);

if (dst == (u_char *) NULL)
	fatalerr("binary_subimage_copy_eq","Null destination image pointer",(char *)NULL);

if ((srcw < 0) || (srch < 0))
	fatalerr("binary_subimage_copy_eq","Negative source image dimension(s)",(char *)NULL);

if ((dstw < 0) || (dsth < 0))
	fatalerr("binary_subimage_copy_eq","Negative destination image dimension(s)",(char *)NULL);

if ((cpw < 0) || (cph < 0))
	fatalerr("binary_subimage_copy_eq","Negative subimage dimension(s)",(char *)NULL);

if (srcw % BITSPERBYTE)
	fatalerr("binary_subimage_copy_eq",
		"Source image width must be a multiple of 8",(char *)NULL);

if (dstw % BITSPERBYTE)
	fatalerr("binary_subimage_copy_eq",
		"Destination image width must be byte aligned",(char *)NULL);

if (((srcx + cpw) > srcw) || ((srcy + cph) > srch))
	fatalerr("binary_subimage_copy_eq",
		"Subimage exceeds source image dimension(s)",(char *)NULL);

if (((dstx + cpw) > dstw) || ((dsty + cph) > dsth))
	fatalerr("binary_subimage_copy_eq",
		"Subimage exceeds destination image dimension(s)",(char *)NULL);


if (!cpw || !cph)
	return;

if (!srcw || !srch)
	return;

if (!dstw || !dsth)
	return;

	i = srcx % BITSPERBYTE;
	i_inv = BITSPERBYTE - i;

j = dstx % BITSPERBYTE;
j_inv = BITSPERBYTE - j;

		b = cpw % BITSPERBYTE;
		b_inv = BITSPERBYTE - b;
		diff = j_inv - b;
		if (diff)
		fatalerr("binary_subimage_copy_eq","bad diff",(char *)NULL);


	srcbw = srcw / BITSPERBYTE;

dstbw = dstw / BITSPERBYTE;

cpbw  = cpw  / BITSPERBYTE;


	/* Increment src to the byte in the upper left corner of the */
	/* subimage to be copied. Increment dst to the upper left corner */
	/* of the byte in the destination that will be modified. */
	src += srcy * srcbw + srcx / BITSPERBYTE;

dst += dsty * dstbw + dstx / BITSPERBYTE;


	/* Set dx (dy) to be the increment needed to get src (dst) to the */
	/* first byte in the next scan line to be copied (or modified) from */
	/* the last byte in the current scan line. */
	dx = srcbw - cpbw;

dy = dstbw - cpbw;


if (!i && !j) {			/* Both src & dst are byte-aligned. */
	m3 = mask_end_1[b_inv];	/* eg. 00000111 */
	m4 = mask_begin_1[b];	/* eg. 11111000 */
	while (cph--) {
		bytes = cpbw;
		while (bytes--)
			*dst++ = *src++;

		*dst = (*dst & m3) | (*src & m4);

		src += dx;
		dst += dy;
	}
	return;
}






if (!j) {			/* Dst is byte-aligned. */
	m3 = mask_end_1[b_inv];	/* eg. 00000111 */
	m4 = mask_begin_1[b];	/* eg. 11111000 */
	while (cph--) {
		bytes = cpbw;
		while (bytes--) {
			c = (*src << i) | (*(src+1) >> i_inv);
			*dst++ = c;
			src++;
		}
		c = (*src << i) | (*(src+1) >> i_inv);
		*dst = (*dst & m3) | (c & m4);
		src += dx;
		dst += dy;
	}
	return;
}








if (!i) {					/* Src is byte-aligned. */
	m1 = mask_end_0[j_inv];				/* eg. 11110000 */
	m2 = mask_begin_0[j];				/* eg. 00001111 */
	if (diff > 0) {
		/* j_inv > b */
		m3 = m1 | mask_end_1[diff];		/* eg. 11110011 */
		m4 = mask_end_0[diff];			/* eg. 11111100 */
	} else if (diff) {
		/* j_inv < b */
		m3 = mask_end_1[BITSPERBYTE+diff];	/* eg. 00111111 */
		m4 = mask_begin_1[-diff];		/* eg. 11000000 */
	}

	while (cph--) {
		bytes = cpbw;
		while (bytes--) {
			c = *src++;
			*dst = (*dst & m1) | (c >> j);
			dst++;
			*dst = (*dst & m2) | (c << j_inv);
		}
		binary_fill_partial(BINARY_COPY,src,i,dst,j,b);
		src += dx;
		dst += dy;
	}
	return;
}






if (i == j) {
	m0 = mask_begin_1[i];
	m1 = mask_end_0[i_inv];
	m2 = mask_end_1[i_inv];
	if (diff > 0) {
		m3 = m1 | mask_end_1[diff];
		m4 = mask_end_0[diff];
	} else if (diff) { 
		m3 = mask_end_1[BITSPERBYTE+diff];
		m4 = mask_begin_1[-diff];
	}

	/* Neither src nor dst is byte-aligned, but they */
	/* are offset by the same amount, so we can copy */
	/* (cpbw - 1) bytes directly from src to dst. The */
	/* bits in the other byte are on the ends of the */
	/* scan line and are treated specially. */

	/* We don't need this case in binary_subimage_*_8(). If cpbw==0, */
	/* then cpw was 0 and we would have already returned. */

	if (! cpbw) {
		while (cph--) {
			binary_fill_partial(BINARY_COPY,src,i,dst,j,b);
			src += dx;
			dst += dy;
		}
		return;
	}

	if (cpbw == 1) {
		while (cph--) {
			*dst = (*dst & m0) | (*src++ & m2);
			dst++;
			*dst = (*dst & m2) | (*src & m0);
			binary_fill_partial(BINARY_COPY,src,i,dst,j,b);
			src += dx;
			dst += dy;
		}
		return;
	}

	while (cph--) {
		*dst = (*dst & m0) | (*src++ & m2);
		dst++;

		bytes = cpbw - 1;
		while (bytes--)
			*dst++ = *src++;

		*dst = (*dst & m2) | (*src & m0);
		binary_fill_partial(BINARY_COPY,src,i,dst,j,b);
		src += dx;
		dst += dy;
	}
	return;
}






m1 = mask_end_0[j_inv];		/* eg. 11111000 */
m2 = mask_begin_0[j];		/* eg. 00000111 */

	if (diff > 0) {
		m3 = m1 | mask_end_1[diff];
		m4 = mask_end_0[diff];
	} else if (diff) { 
		m3 = mask_end_1[BITSPERBYTE+diff];
		m4 = mask_begin_1[-diff];
	}

while (cph--) {				/* The absolute worst case. Neither */
	bytes = cpbw;			/* src nor dst is byte-aligned, and */
	while (bytes--) {		/* they are offset by diff. amounts. */
		c = (*src << i) | (*(src+1) >> i_inv);
		*dst = (*dst & m1) | (c >> j);
		dst++;
		*dst = (*dst & m2) | (c << j_inv);
		src++;
	}
	binary_fill_partial(BINARY_COPY,src,i,dst,j,b);
	src += dx;
	dst += dy;
}
}
/* LINTLIBRARY */

#include <stdio.h>
#include <sys/types.h>





/************************************************************/
/*                                                          */
/*         Routines:  binary_subimage_copy()                */
/*                    binary_subimage_or()                  */
/*                    binary_subimage_and()                 */
/*                    binary_subimage_xor()                 */
/*                    binary_subimage_invert()              */
/*                    binary_subimage_zero()                */
/*                    binary_subimage_one()                 */
/*                                                          */
/*         Author:    Stan Janet                            */
/*         Date:      11/16/90                              */
/*                                                          */
/* binary_subimage_copy() is an all-purpose bit-level copy  */
/*    utility.                                              */
/* If the subimage to be copied is a multiple of 8 bits     */
/*    wide, it calls binary_subimage_copy_8(). Otherwise it */
/*    calls one binary_subimage_copy_{gt,lt,eq}().          */
/* binary_subimage_copy_{gt,lt,eq}() are bit-level copy     */
/*    utilities for subimages that are not a multiple of 8  */
/*    bits wide and where the space remaining in the last   */
/*    byte in every destination scan line will be (respect- */
/*    ively) greater than, less than, or equal to the num-  */
/*    ber of bits by which the subimage width exceeds 8.    */
/* binary_subimage_{or,and,xor,invert}() are all-purpose    */
/*    bit-level logical or, and, xor, and invert utilities  */
/*    that works analogously to the copier described above. */
/* binary_subimage_{zero,one}() are all-purpose bit-level   */
/*    utilities to set all bits in a subimage to zero and   */
/*    one, respectively.                                    */
/* All dimensions are taken to be in bit units.             */
/************************************************************/

void binary_subimage_copy (
	u_char *src, int  srcw,int srch,
	u_char  *dst, int dstw, int dsth,
       int srcx,int srcy,
       int  cpw,int cph,
       int dstx,int dsty)
{
register int j_inv, b, diff;

if (src == (u_char *) NULL)
	fatalerr("binary_subimage_copy",
		"Null source image pointer",(char *)NULL);
if (dst == (u_char *) NULL)
	fatalerr("binary_subimage_copy",
		"Null destination image pointer",(char *)NULL);
if ((srcw < 0) || (srch < 0))
	fatalerr("binary_subimage_copy",
		"Negative source image dimension(s)",(char *)NULL);
if ((dstw < 0) || (dsth < 0))
	fatalerr("binary_subimage_copy",
		"Negative destination image dimension(s)",(char *)NULL);
if ((cpw < 0) || (cph < 0))
	fatalerr("binary_subimage_copy",
		"Negative subimage dimension(s)",(char *)NULL);
if (srcw % BITSPERBYTE)
	fatalerr("binary_subimage_copy",
		"Source data width must be a multiple of 8",(char *)NULL);
if (dstw % BITSPERBYTE)
	fatalerr("binary_subimage_copy",
		"Destination data width must be a multiple of 8",(char *)NULL);
if (((srcx + cpw) > srcw) || ((srcy + cph) > srch))
	fatalerr("binary_subimage_copy",
		"Source bit addresses exceed data range",(char *)NULL);
if (((dstx + cpw) > dstw) || ((dsty + cph) > dsth))
	fatalerr("binary_subimage_copy",
		"Destination bit addresses exceed data range",(char *)NULL);

if (!cpw || !cph)
	return;

if (!srcw || !srch)
	return;

if (!dstw || !dsth)
	return;

b = cpw % BITSPERBYTE;
if (!b) {
	binary_subimage_copy_8(src,srcw,srch,
				dst,dstw,dsth,
				srcx,srcy,cpw,cph,dstx,dsty);
	return;
}

j_inv = BITSPERBYTE - dstx % BITSPERBYTE;
diff = j_inv - b;

if (diff > 0) {
	binary_subimage_copy_gt(src,srcw,srch,
				dst,dstw,dsth,
				srcx,srcy,cpw,cph,dstx,dsty);
	return;
}

if (diff < 0) {
	binary_subimage_copy_lt(src,srcw,srch,
				dst,dstw,dsth,
				srcx,srcy,cpw,cph,dstx,dsty);
	return;
}

	binary_subimage_copy_eq(src,srcw,srch,
				dst,dstw,dsth,
				srcx,srcy,cpw,cph,dstx,dsty);
}
