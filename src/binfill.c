/***********************************************************************
      LIBRARY: IMAGE - Image Manipulation and Processing Routines

      FILE:    BINFILL.C

      AUTHORS: Stan Janet
      DATE:    11/16/1990

      Contains routines responsible for filling a specified subimage
      of a binary image bitmap.

      ROUTINES:
#cat: binary_fill_partial - uses a logical operator to copy pixels from a
#cat:                       location in one binary scanline to a location in
#cat:                       another binary scanline.
#cat: gb - gets a pixel from a binary scanline.
#cat:
#cat: sb - sets a pixel in a binary scanline.
#cat:

***********************************************************************/

/* LINTLIBRARY */

#include <stdio.h>
#include <limits.h>
#include <sys/types.h>
#include <binops.h>
#include <copy.h>
#include <ffpis/util/util.h>

#ifndef BITSPERBYTE
#define BITSPERBYTE CHAR_BIT
#endif


static int gb( u_char *p, int i)
{

p += i / BITSPERBYTE;
i %= BITSPERBYTE;
return (*p >> ((BITSPERBYTE - 1) - i)) & 0x1;
}

static void sb( u_char *p, int i, int v)
{
u_char m;

p += i / BITSPERBYTE;
i %= BITSPERBYTE;
m = 0x1 << ((BITSPERBYTE - 1) - i);
if (v)
	*p |= m;
else
	*p &= ~m;
}

void binary_fill_partial( int op, u_char *src,int  i,u_char  *dst, int  j,
		int  bits)
{
int n;
/*
int i_inv;
int j_inv;
int diff;
u_char sc;
u_char src_begin, src_end;
u_char dst_begin, dst_end;

i_inv = BITSPERBYTE - i;
j_inv = BITSPERBYTE - j;

src_begin = mask_begin_1[i];
src_end   = mask_end_1[i_inv];

dst_begin = mask_begin_1[j];
dst_end   = mask_end_1[j_inv];


if (i + bits <= BITSPERBYTE)
	sc = *src << i;
else
	sc = (*src << i) | (*(src+1) >> i_inv);
*/

for (n=0; n<bits; n++) {
	int srcbit, dstbit;

	switch (op) {

		case BINARY_COPY:
			srcbit = gb(src,i+n);
			sb(dst,j+n,srcbit);
			break;

		case BINARY_OR:
			srcbit = gb(src,i+n);
			dstbit = gb(dst,j+n);
			sb(dst,j+n,srcbit|dstbit);
			break;

		case BINARY_AND:
			srcbit = gb(src,i+n);
			dstbit = gb(dst,j+n);
			sb(dst,j+n,srcbit&dstbit);
			break;

		case BINARY_XOR:
			srcbit = gb(src,i+n);
			dstbit = gb(dst,j+n);
			sb(dst,j+n,srcbit^dstbit);
			break;

		case BINARY_INVERT:
			srcbit = gb(src,i+n);
			sb(dst,j+n,~srcbit);
			break;  

		case BINARY_ONE:
			sb(dst,j+n,1);
			break;  

		case BINARY_ZERO:
			sb(dst,j+n,0);
			break;  

		default:
			fatalerr("binary_fill_partial","bad operator",NULL);
			/* FIXME What should be third string */
	} /* SWITCH */
} /* FOR */

/*
diff = bits - j;
m = mask_begin_1[(j+bits) % BITSPERBYTE];
switch (op) {

  case BINARY_COPY:
	if (diff > 0) {
		*dst = (*dst & dst_begin) | ((sc >> j) & dst_end);
		dst++;
		*dst = (*dst & ~m) | ((sc << j_inv) & m);
		dst--;
	} else if (diff < 0)
		*dst = (*dst & dst_begin) | 
	else
		*dst = (*dst & dst_begin) | ((sc >> j) & dst_end);
	break;

  case BINARY_OR:
		break;
'
  case BINARY_AND:
		break;

  default:
		fatalerr("binary_fill_partial","bad opcode",(char *)NULL);
}
*/
}
