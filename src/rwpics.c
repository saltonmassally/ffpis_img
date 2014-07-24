/************************************************************************

      PACKAGE:  PCASYS TOOLS

      FILE:     RWPICS.C

      AUTHORS:  Craig Watson
                cwatson@nist.gov
                G. T. Candela
      DATE:     08/01/1995

#cat: rwpics - Converts regional weight data to an image. Three modes
#cat:          convert the regional weights, basepoints, or the
#cat:          estimated gradients at the basepoints.

Makes grayscale pictures suitable for examining rwxrh matrices
associated with optimization of the regional weights: either the
basepoints, i.e. values of the regional weights at the outermost
iterations of the optimizer (optrws), or the estimated gradients at
the basepoints.

There are three usage modes, as specified by the rws|eg|seg argument:

If rws ("regional weights"), the program makes a grayscale picture
that is reasonable if the input file represents a set of regional
weights, e.g. one of the bspt files produced by an optrws run.  To do
so, it linearly maps absolute values of input values to gray tones,
setting the mapping so that 0. maps to black and the maximum absolute
value across all components of all input matrices, to white.  Absolute
values are the reasonable thing to depict when examining a point in
regional-weights space, since the sign of a regional weights has no
effect on the NN classifier.  (Optimization may sometimes cause some
unimportant outer weights to be slightly negative.)  The mapping is
adapted to the maximum absolute value across _all_ input files, rather
than being separately adapted for each input file; this is done so
that the several resulting pictures can be examined side by side with
the knowledge that all gray tones are on the same scale.

If eg ("estimated gradient"), the program makes a grayscale picture
that is reasonable if the input file represents an estimated gradient
of the error function, e.g. one of the egrad files produced by an
optrws run.  To do so, it affinely maps input values to gray tones,
setting the mapping so that the minimum input value across all input
files is mapped to white and the maximum input value, to black.

If seg ("sign of estimated gradient"), the program makes a binary
picture that is reasonable if the input file represents an estimated
gradient of the error function.  To do so, it maps negative values to
white and nonnegative values to black.  This is interesting because if
the component of the estimated gradient (i.e., the estimated partial
derivative) associated with a region is negative, that shows that the
weight for this region should be increased (and will be increased by
optrws).

The program makes its output image files in IHEAD format, producing
them in directory outpics_dir.  Each output file's name is produced by
taking the last component of the associated input file and appending
an underscore, then the rws|eg|seg argument value, then .pct (the
standard IHEAD file suffix).

*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ffpis/util/little.h>
#include <ffpis/util/usagemcs.h>
#include <ffpis/util/util.h>
#include <ffpis/util/datafile.h>
#include <ffpis/util/memalloc.h>

#define RWS 0
#define EG 1
#define SEG 2
#define PWS 15

int debug =0;

int main(int argc, char *argv[])
{
  char *rws_eg_seg, *outpics_dir, str[400], *desc;
  unsigned char pixval;
  static unsigned char *pic;
  int mode, iarg, i, ii, iis, iie, j, jj, jjs, jje;
  float *buf, *p, *pe, aval;
  float minval=9999.9, maxval=-9999.9, range, a=0.0, b=0.0, maxabs, c;
  int rw, rh;
  int pw, ph;

  if(argc < 4)
    usage("<rwfile_in[rwfile_in...]> <rws|eg|seg> <outpics_dir>");
  rws_eg_seg = argv[argc - 2];
  outpics_dir = argv[argc - 1];
  if(!strcmp(rws_eg_seg, "rws"))
    mode = RWS;
  else if(!strcmp(rws_eg_seg, "eg"))
    mode = EG;
  else if(!strcmp(rws_eg_seg, "seg"))
    mode = SEG;
  else
    fatalerr("the rws|eg|seg arg must be rws, eg, or seg", (char *)NULL,
             (char *)NULL);
  if(mode != SEG) {
    for(iarg = 1; iarg < argc - 2; iarg++) {
      matrix_read(argv[iarg], &desc, &rh, &rw, &buf);
      free(desc);
      for(pe = (p = buf) + rw*rh; p < pe; p++) {
	aval = *p;
	if(iarg == 1 && p == buf)
	  minval = maxval = aval;
	else if(aval < minval)
	  minval = aval;
	else if(aval > maxval)
	  maxval = aval;
      }
      free(buf);
    }
    if(mode == EG) {
      range = maxval - minval;
      if(range > 0.) {
	a = -255. / range;
	b = 255. * maxval / range;
      }
      else {
	a = 0.;
	b = 128.;
      }
    }
    else {
      maxabs = fabs((double)minval);
      if((c = fabs((double)maxval)) > maxabs)
	maxabs = c;
      if(maxabs > 0.) {
	a = 255. / maxabs;
	b = 0.;
      }
      else {
	a = 0.;
	b = 128.;
      }
    }
  }
  for(iarg = 1; iarg < argc - 2; iarg++) {
    matrix_read(argv[iarg], &desc, &rh, &rw, &buf);
    free(desc);
    pw = PWS * rw;
    ph = PWS * rh;
    malloc_uchar(&pic, pw*ph, "rwpics pic");
    for(i = iis = 0, iie = PWS, p = buf; i < rh; i++, iis += PWS,
      iie += PWS)
      for(j = jjs = 0, jje = PWS; j < rw; j++, jjs += PWS, jje += PWS,
        p++) {
	aval = *p;
	if(mode == EG)
	  pixval = a * aval + b + .5;
	else if(mode == SEG)
	  pixval = (aval < 0. ? 255 : 0);
	else
	  pixval = a * fabs((double)aval) + b + .5;
	for(ii = iis; ii < iie; ii++)
	  for(jj = jjs; jj < jje; jj++)
	    pic[ii*pw+jj] = pixval;
      }
    free(buf);
    sprintf(str, "%s/%s_%s.pct", outpics_dir, lastcomp(argv[iarg]),
      rws_eg_seg);
    write_ihdr_std(pic, pw, ph, 8, str);
    free(pic);
  }
  return 0;
}

void print_usage(const char *s)
{
    (void)s;
    fprintf(stderr,"<rwfile_in[rwfile_in...]> <rws|eg|seg> <outpics_dir>\n");
}
