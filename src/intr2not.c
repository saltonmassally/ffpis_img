/************************************************************************

      PACKAGE:  IMAGE ENCODER/DECODER TOOLS

      FILE:     INTR2NOT.C

      AUTHORS:  Craig Watson
                cwatson@nist.gov
                Michael Garris
                mgarris@nist.gov
      DATE:     11/17/2000

#cat: intr2not - Takes an IHead or raw pixmap with interleaved color
#cat:            components.  It de-interleaves the pixmap creating
#cat:            one 8-bit image plane for each color component.  This
#cat:            can handle specific downsamplings of component planes
#cat:            that are commonly used with YCbCr colorspace images.

*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/param.h>
#include <intrlv.h>
#include <ihead.h>
#include <getnset.h>
#include <img_io.h>
#include <parsargs.h>
#include <ffpis/util/ioutil.h>


void procargs(int, char **, char **, char **, int *, int *, int *,
              int *, int *, int *, int *, int *);
void print_usage(char *);

int debug = 0;

/******************/
/*Start of Program*/
/******************/

int main(int argc, char *argv[])
{
   int ret, rawflag;
   char *outext;                   /* ouput file extension */
   char *ifile, ofile[MAXPATHLEN]; /* file names */
   int width, height, depth, ppi, ilen, olen;  /* image parameters */
   IHEAD *ihead;
   unsigned char *idata, *odata;           /* image pointers */
   int hor_sampfctr[MAX_CMPNTS], vrt_sampfctr[MAX_CMPNTS];
   int n_cmpnts;


   procargs(argc, argv, &outext, &ifile, &rawflag,
            &width, &height, &depth, &ppi,
            hor_sampfctr, vrt_sampfctr, &n_cmpnts);

   /* If H,V's are specified on command line, then YCbCr ... */
   /* so read a raw input file. */
   if(argc == 7){
      ret = read_raw_from_filesize(ifile, &idata, &ilen);
      if(ret)
         exit(ret);

      ret = test_image_size(ilen, width, height, hor_sampfctr,
		      vrt_sampfctr, n_cmpnts, 1);
      if(ret)
         exit(ret);
   }
   else{
      ret = read_raw_or_ihead(!rawflag, ifile, &ihead, &idata, &width, &height, &depth);
      if(ret)
         exit(ret);
   }

   if(debug > 0)
      fprintf(stdout, "File %s read\n", ifile);


   /* If IHead image file ... */
   if(!rawflag){
      /* Get PPI from IHead. */
      ppi = get_density(ihead);
      free(ihead);
   }

   ret = intrlv2not_mem(&odata, &olen, idata, width, height, depth,
		   hor_sampfctr, vrt_sampfctr, n_cmpnts);
   if(ret){
      free(idata);
      exit(ret);
   }
   free(idata);

   if(debug > 0)
      fprintf(stdout, "Image data converted to non-interleaved\n");

   fileroot(ifile);
   sprintf(ofile, "%s.%s", ifile, outext);

   ret = write_raw_from_memsize(ofile, odata, olen);
   if(ret){
      free(odata);
      exit(ret);
   }

   if(debug > 0)
      fprintf(stdout, "Image data written to file %s\n", ofile);

   free(odata);

   exit(0);
}

/*****************************************************************/
void procargs(int argc, char **argv,
              char **outext, char **ifile, int *rawflag,
              int *width, int *height, int *depth, int *ppi,
              int *hor_sampfctr, int *vrt_sampfctr, int *n_cmpnts)
{
   int argi, n_cmpnts_tst;

   if((argc < 3) || (argc > 7)){
      print_usage(argv[0]);
      exit(-1);
   }

   *outext = argv[1];
   *ifile = argv[2];
   *rawflag = 0;
   *width = -1;
   *height = -1;
   *depth = -1;
   *ppi = -1;
   hor_sampfctr[0] = 1;
   vrt_sampfctr[0] = 1;
   hor_sampfctr[1] = 1;
   vrt_sampfctr[1] = 1;
   hor_sampfctr[2] = 1;
   vrt_sampfctr[2] = 1;
   *n_cmpnts = 3;

   if(argc == 3)
      return;
   argi = 3;
   /* If rawflag ... */
   if(strncmp(argv[argi], "-r", 2) == 0){
      *rawflag = 1;
      argi++;
      if(argi == argc){
         print_usage(argv[0]);
         fprintf(stderr,
            "       expected \"w,h,d,[ppi]\" following \"-raw\" option\n");
         exit(-1);
      }
      parse_w_h_d_ppi(argv[argi], argv[0], width, height, depth, ppi);
      /* We are requiring 3 components with each component having 8 bits. */
      /* Thus pixel depth must be 24.  The number of permitted components */
      /* could be changed, for example extended to 4, but this            */
      /* implementation requires 8 bits per component regardless.         */
      if(*depth != 24){
         print_usage(argv[0]);
         fprintf(stderr, "       pixel depth = %d must be 24\n", *depth);
         exit(-1);
      }
      argi++;
      if(argi == argc)
         return;

      /* If YCbCr flag ... */
      if(strncmp(argv[argi], "-Y", 2) == 0){
         argi++;
         if(argi == argc){
            print_usage(argv[0]);
            fprintf(stderr,
                    "       H,V sample factors required with -YCbCr\n");
            exit(-1);
         }
         parse_h_v_sampfctrs(argv[argi], argv[0],
                             hor_sampfctr, vrt_sampfctr, &n_cmpnts_tst);
         if(n_cmpnts_tst != 3) {
            print_usage(argv[0]);
            fprintf(stderr, "       expecting 3 components\n");
            exit(-1);
         }
         argi++;
      }
   }

   if(argi != argc){
      print_usage(argv[0]);
      fprintf(stderr, "       Invalid argument list\n");
      exit(-1);
   }
}

/*****************************************************************/
void print_usage(char *arg0)
{
   fprintf(stderr, "Usage: %s <outext> <image file>\n", arg0);
   fprintf(stderr, "                  [-raw_in w,h,d,[ppi]\n");
   fprintf(stderr, "                     [-YCbCr H0,V0:H1,V1:H2,V2]]\n");
}
