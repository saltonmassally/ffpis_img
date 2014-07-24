/************************************************************************

      PACKAGE:  PCASYS TOOLS

      FILE:     FIXWTS.C

      AUTHORS:  Craig Watson
                cwatson@nist.gov
                Charles Wilson
                cwilson@nist.gov
      DATE:     10/01/2000

#cat: fixwts - M-weighted robust weight filter from network
#cat:          activations.

*************************************************************************/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <ffpis/util/usagemcs.h>
#include <ffpis/util/little.h>
#include <ffpis/util/util.h>
#include <ffpis/util/memalloc.h>

#define	CLIM	9.0	/* robust limit value */
static float wtfunc( float *z)
/* weight function for robust M-weights after Andrews (1973)   */

{
   float a, b;

   if ((*z < -CLIM)|(*z > CLIM))
      return 0.0;
   else {
      a = *z/CLIM;
      b = 1.0-a*a;
      return (a*b*b);
   }
}

/* medk.c 25-Dec-86 20:50     from JLB
   finds k-th smallest number in array x. rearranges x.
   algorithm from Wirth
*/

static float medk(int n,float x[],int  k)
{
   int i, j, left, right;
   double middle;
    
   if (n < k || k < 1)
      return x[0];
    
   left = 0;
   right = n - 1;
    
   while (left < right) {
      middle = x[k];
      for (i = left, j = right ; i <= j ; ) {
         while (x[i] < middle)
            i++;
         while (middle < x[j])
            j--;
         if (i <= j) {
            float temp;
            temp = x[i];
            x[i] = x[j];
            x[j] = temp;
            i++;
            j--;
         }
      }
      if (j < k) 
         left = i;
      if (k < i) 
         right = j;
    }
    return x[k];
}
/* end of medk.c */

int main(int argc, char *argv[])
{	
   int npats, ninps, nhids, nouts;
   char line[1000], junk[100];
   float w, *vout, *r, *yr;
   int i, j, k, k1;
   float a, b, s;
   double sum;
   FILE *fp;

   if(argc != 3)
      usage("<long_error_file> <output_pat_wts>");

   if((fp = fopen(argv[1], "rb")) == NULL)
      syserr("fixwts", "fopen", argv[1]);

   fscanf(fp, "%d %d %d %d", &npats, &ninps, &nhids, &nouts);
   fgets(line,999,fp);
   fgets(line,999,fp);

   malloc_flt(&vout, nouts, "fixwts vout");
   malloc_flt(&r, npats+1, "fixwts r");
   malloc_flt(&yr, npats+1, "fixwts yr");

   k1 = npats/2;
   for (i = 0; i <= npats; i++) {
      fscanf(fp, "%s %s %s %s %d", junk, junk, junk, junk, &j);
      sum = 0.0;
      for (k = 0; k < nouts; k++) {
         fscanf(fp, "%e", &vout[k]);
         if (k == (j-1))
            sum = sum + 1.0 - 2.0 * vout[k] + vout[k] * vout[k];
         else
            sum = sum + vout[k] * vout[k];
      }
      r[i] = sqrt(sum);
      yr[i] = fabs( (double) r[i]);
   }
   fclose(fp);
   free(vout);

   if((fp = fopen(argv[2], "wb")) == NULL)
      syserr("fixwts", "fopen", argv[2]);

   s = medk(npats,yr,k1);
   for (i = 0; i <= npats; i++) {
      yr[i] = r[i];
      if (r[i] != 0.0) {
         a = r[i]/s;
         b = wtfunc(&a);
         w = sqrt( (double) (b/r[i]));
      }
      else
         w = 1.0;

      fprintf(fp, "%10.7f\n",w);
   }
   fclose(fp);
   free(r);
   free(yr);
   exit(0);
}
