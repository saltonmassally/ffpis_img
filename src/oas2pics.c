/************************************************************************

      PACKAGE:  PCASYS TOOLS

      FILE:     OAS2PICS.C

      AUTHORS:  Craig Watson
                cwatson@nist.gov
                G. T. Candela
      DATE:     08/01/1995

#cat: oas2pics - Converts orientation arrays into gray/binary
#cat:            (ie. 0|255) IHEAD raster images.

Reads orientation arrays (oa's) i_start through i_finish (numbered
starting at 1) from oasfile, which must be a pcasys "matrix" format
file with second dimension 2*aw*ah.  (Each row of the matrix is one
orientation array, with all the x components or orientation vectors
first in row-major order, then all the y components.)  Makes a
gray/binary (ie. 0 | 255) IHEAD raster image depicting each oa, and
produces these image files in outpics_dir, with names <i_start>.pct,
<i_start + 1>.pct, ..., <i_finish>.pct.  Creates outpics_dir if it does
not already exist.  The picture of an oa shows each of its orientation
vectors as a bar, which should be approximately parallel to ridges and
valleys in its vicinity if the oa has been produced well.  If verbose
is y, then the program writes progress messages to stdout.

*************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <ffpis/util/usagemcs.h>
#include <ffpis/util/little.h>
#include <ffpis/util/datafile.h>
#include <ffpis/util/memalloc.h>
#include <ffpis/util/util.h>
//#include <ffpis/util/pca.h>
enum Standard_Image_Size { WIDTH=512, HEIGHT=480 };
enum window_sizes { WS=16, HWS=8 };

void dmt_to_pic(float **dg,float ** mag,int aw,int  ah,
		float  top,unsigned char *pic,int  w,int  h);
void xy_to_dmt(float **x,float ** y,float ** dg,float ** mag,
		int w,int h,float * top_ret);

int debug=0;

void print_usage(const char *s)
{
  const char estr[]="%s <oasfile> <i_start> <i_finish> <outpics_dir verbose>";
  fprintf(stderr,estr,s);
}

int main(int argc, char *argv[])
{
  FILE *fp;
  char *oasfile, *outpics_dir, *verbose, *desc, str[200];
  static unsigned char *bytes_pic;
  int w, h;
  int i_start, i_finish, i, ascii, dim1, dim2, isverbose;
  float top;
  static float **x, **y, *oa, **dg, **mag, *xp, *yp;
  int aw, ah;
  int i2, j2;

  Usage("<oasfile> <i_start> <i_finish> <outpics_dir verbose>");
  oasfile = *++argv;
  i_start = atoi(*++argv);
  i_finish = atoi(*++argv);
  outpics_dir = *++argv;
  verbose = *++argv;
  if(!strcmp(verbose, "y"))
     isverbose = 1;
  else if(!strcmp(verbose, "n"))
     isverbose = 0;
  else
    fatalerr("oas2pics", "verbose must be y or n", NULL);
  if(i_start < 1)
    fatalerr("oas2pics", "i_start must be >= 1", NULL);
  if(i_start > i_finish)
    fatalerr("oas2pics", "i_start must be <= i_finish", NULL);
  mkdir(outpics_dir, 0700);

  w = WIDTH;
  h = HEIGHT;
  aw = w/WS-2;
  ah = h/WS-2;

  malloc_uchar(&bytes_pic, h*w, "oas2pics bytes_pic");
  malloc_dbl_flt(&dg, ah, aw, "oas2pics dg");
  malloc_dbl_flt(&mag, ah, aw, "oas2pics mag");
  malloc_dbl_flt(&x, ah, aw, "oas2pics x");
  malloc_dbl_flt(&y, ah, aw, "oas2pics y");
  malloc_flt(&oa, 2*aw*ah, "oas2pics oa");

  matrix_readrow_init(oasfile, &desc, &ascii, &dim1, &dim2, &fp);
  free(desc);
  if(dim2 != 2*aw*ah) {
    sprintf(str, "second dimension, %d, of %s is not %d", dim2,
      oasfile, 2*aw*ah);
    fatalerr("oas2pics", str, NULL);
  }
  if(i_finish > dim1) {
    sprintf(str, "i_finish is %d, but %s has only %d oa's", i_finish,
      oasfile, dim1);
    fatalerr("oas2pics", str, NULL);
  }
  for(i = 1; i < i_start; i++)
    matrix_readrow(fp, ascii, dim2, oa);
  for(i = i_start; i <= i_finish; i++) {
    if(isverbose)
      printf("%d (%d - %d)\n", i, i_start, i_finish);
    matrix_readrow(fp, ascii, dim2, oa);

    xp = oa;
    yp = oa + (aw*ah);
    for(j2 = 0; j2 < ah; j2++)
      for(i2 = 0; i2 < aw; i2++) {
         x[j2][i2] = *xp++;
         y[j2][i2] = *yp++;
      }

    xy_to_dmt(x, y, dg, mag, aw, ah, &top);
    dmt_to_pic(dg, mag, aw, ah, top, bytes_pic, w, h);
    sprintf(str, "%s/%d.pct", outpics_dir, i);
    write_ihdr_std(bytes_pic, w, h, 8, str);
  }
  free(oa);
  free(bytes_pic);
  free_dbl_flt(dg, ah);
  free_dbl_flt(mag, ah);
  free_dbl_flt(x, ah);
  free_dbl_flt(y, ah);
  return 0;
}

/*******************************************************************/
void xy_to_dmt(float **x,float ** y,float ** dg,float ** mag,
		int w,int h,float * top_ret)
{
  int i, j;
  float top, themag;
  static float a = 28.6479;  /* 90./pi */

  for(i = 0, top = -1.; i < h; i++)
    for(j = 0; j < w; j++) {
      if(x[i][j] > 0.) {
	if(y[i][j] >= 0.)
	  dg[i][j] = a * atan((double)(y[i][j] / x[i][j]));
	else
	  dg[i][j] = a * atan((double)(y[i][j] / x[i][j])) + 180.;
      }
      else if(x[i][j] == 0.) {
	if(y[i][j] > 0.)
	  dg[i][j] = 45.;
	else if(y[i][j] < 0.)
	  dg[i][j] = 135.;
	else
	  dg[i][j] = 0.;
      }
      else
	dg[i][j] = a * atan((double)(y[i][j] / x[i][j])) + 90.;
      if((themag = mag[i][j] = sqrt((double)(x[i][j] * x[i][j] +
        y[i][j] * y[i][j]))) > top)
	top = themag;
    }
  *top_ret = top;
}

/*******************************************************************/
void dmt_to_pic(float **dg,float ** mag,int aw,int  ah,
		float  top,unsigned char *pic,int  w,int  h)
{
  int i, j, iangle, ii, jj, ilen, jjlo, jjhi, iyow, jyow;
  float angle, ac, as, x0[WS][WS], y0[WS][WS], hurl;
  static int f = 1, bars[WS][WS][32][4];
  static float del = .09817 /* pi/32. */, fac = .177778 /* 32./180.*/;
  unsigned char *ptr, *tptr;
  float hwsp5;
  int jstp;

  if(f) {
    f = 0;
    if(HWS < 4 || HWS%4)
       fprintf(stdout, "Blocksize (HWS) should be >= 4 and even divisible by 4\n");
    hwsp5 = (float)HWS - 0.5;
    jstp = HWS/4;
    for(i = 0; i < WS; i++)
      for(j = 0; j < WS; j++) {
        x0[i][j] = (float)j - hwsp5;
        y0[i][j] = hwsp5 - (float)i;
      }
    for(iangle = 0, angle = 0.; iangle < 32; iangle++, angle += del) {
      ac = cos((double)angle);
      as = sin((double)angle);
      for(i = 0; i < WS; i++)
        for(j = 0; j < WS; j++) {
           hurl = ac * x0[i][j] + as * y0[i][j] + hwsp5;
           jj = ((hurl >= 0.) ? hurl + .5 : hurl - .5);
           hurl = as * x0[i][j] - ac * y0[i][j] + hwsp5;
           ii = ((hurl >= 0.) ? hurl + .5 : hurl - .5);
           jjlo = HWS-jstp;
           jjhi = HWS+jstp-1;
           for(ilen = 0; ilen < 4; ilen++, jjlo -= jstp, jjhi += jstp)
             bars[i][j][iangle][ilen] = (ii >= HWS-1 && ii <= HWS
               && jj >= jjlo && jj <= jjhi);
         }
    }
  }
  for(i = 0; i < w*h; i++)
    pic[i] = 0;

  for(i = 0, iyow = WS; i < ah; i++, iyow += WS)
    for(j = 0, jyow = WS; j < aw; j++, jyow += WS) {
      ilen = 3. * mag[i][j] / top + .5;
      if(ilen == 0)
        continue;
      iangle = (int)(dg[i][j] * fac + .5);
      if(iangle == 32)
        iangle = 0;

      tptr = pic + (iyow*w) + jyow;
      for(ii = 0; ii < WS; ii++) {
        ptr = tptr;
	for(jj = 0; jj < WS; jj++) {
	  if(bars[ii][jj][iangle][ilen])
            *ptr = 255;
          ptr++;
        }
        tptr += w;
      }
    }
}

/********************************************************************/
