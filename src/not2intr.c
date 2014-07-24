/************************************************************************

      PACKAGE:  IMAGE ENCODER/DECODER TOOLS

      FILE:     NOT2INTR.C

      AUTHORS:  Craig Watson
                cwatson@nist.gov
                Michael Garris
                mgarris@nist.gov
      DATE:     11/17/2000

#cat: not2intr - Takes an IHead or raw pixmap with non-interleaved color
#cat:            components.  It interleaves the pixel color components
#cat:            into a single 24-bit plane.  This can handle specific
#cat:            downsamplings of component planes that are commonly used
#cat:            with YCbCr colorspace images.

*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/param.h>
#include <intrlv.h>
#include <img_io.h>
#include <ffpis/util/ioutil.h>
#include <parsargs.h>

void procargs(int, char **, char **, char **, int *, int *, int *,
              int *, int *, int *, int *, int *);
void print_usage(char *);

int debug = 0;

/******************/
/*Start of Program*/
/******************/

int main(int argc, char *argv[])
{
   int ret, n, rawflag;
   char *outext;                   /* ouput file extension */
   char *ifile, ofile[MAXPATHLEN]; /* file names */
   FILE *outfp;             /* output file pointer */
   int  width, height, depth, ppi, ilen, olen;  /* image parameters */
   unsigned char *idata, *odata;           /* image pointers */
   int hor_sampfctr[MAX_CMPNTS], vrt_sampfctr[MAX_CMPNTS];
   int n_cmpnts;
   int fsize;


   procargs(argc, argv, &outext, &ifile,
            &width, &height, &depth, &ppi, &rawflag,
            hor_sampfctr, vrt_sampfctr, &n_cmpnts);

   if((ret = filesize(ifile)) < 0)
      exit(ret);
   fsize = ret;

   ret = test_image_size(fsize, width, height, hor_sampfctr, vrt_sampfctr, n_cmpnts, 0);
   if(ret)
      exit(ret);

   ret = read_raw_from_filesize(ifile, &idata, &ilen);
   if(ret)
      exit(ret);

   if(debug > 0)
      fprintf(stdout, "File %s read\n", ifile);


   ret = not2intrlv_mem(&odata, &olen, idata, width, height, depth,
		   hor_sampfctr, vrt_sampfctr, n_cmpnts);
   if(ret){
      free(idata);
      exit(ret);
   }
   free(idata);

   if(debug > 0)
      fprintf(stdout, "Image data converted to interleaved\n");

   /* Write interleaved image file. */
   fileroot(ifile);
   sprintf(ofile, "%s.%s", ifile, outext);

   /* If H,V's are specified on command line, then YCbCr ... */
   /* so write a raw output file. */
   if(argc == 7){
      if((outfp = fopen(ofile, "wb")) == NULL) {
         fprintf(stderr, "ERROR: main : fopen : %s\n",ofile);
         free(odata);
         exit(-5);
      }

      if((n = fwrite(odata, 1, olen, outfp)) != olen){
         fprintf(stderr, "ERROR: main : fwrite : ");
         fprintf(stderr, "only %d of %d bytes written from file %s\n",
                 n, olen, ofile);
         free(odata);
         exit(-6);
      }
      fclose(outfp);
   }
   /* Otherwise, H,V's not specified on command line, so component planes */
   /* are all the same size ... so "possibly" write an IHead file. */
   else{
      ret = write_raw_or_ihead(!rawflag, ofile, odata, width, height, depth, ppi);
      if(ret){
         free(odata);
         exit(ret);
      }
   }

   if(debug > 0)
      fprintf(stdout, "Image data written to file %s\n", ofile);

   free(odata);

   exit(0);
}

/*****************************************************************/
void procargs(int argc, char **argv,
              char **outext, char **ifile,
              int *width, int *height, int *depth, int *ppi, int *rawflag,
              int *hor_sampfctr, int *vrt_sampfctr, int *n_cmpnts)
{
   int argi, n_cmpnts_tst;

   if((argc < 4) || (argc > 7)){
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

   /* Argc must == 4, so parse "w,h,d,[ppi]" */
   argi = 3;
   parse_w_h_d_ppi(argv[argi], argv[0], width, height, depth, ppi);

   /* We are requiring 3 components with each component having 8 bits.    */
   /* Thus pixel depth must be 24.  The number of permitted components    */
   /* could be changed, for example exteded to 4, but this implementation */
   /* requires 8 bits per component regardless.                           */
   if(*depth != 24){
      print_usage(argv[0]);
      fprintf(stderr, "       pixel depth = %d must be 24\n", *depth);
      exit(-1);
   }
   argi++;
   if(argi == argc)
      return;

   /* If rawflag ... */
   if(strncmp(argv[4], "-r", 2) == 0){
      *rawflag = 1;
      argi++;
      if(argi == argc)
         return;
   }
   /* If YCbCr flag ... */
   if(strncmp(argv[argi], "-Y", 2) == 0){
      /* rawflag must be set */
      if(*rawflag == 0){
         print_usage(argv[0]);
         fprintf(stderr, "       -raw_out must be used with -YCbCr\n");
         exit(-1);
      }
      argi++;
      if(argi == argc){
         print_usage(argv[0]);
         fprintf(stderr, "       H,V sample factors required with -YCbCr\n");
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

   if(argi != argc){
      print_usage(argv[0]);
      fprintf(stderr, "       Invalid argument list\n");
      exit(-1);
   }
}

/*****************************************************************/
void print_usage(char *arg0)
{
   fprintf(stderr, "Usage: %s <outext> <image file> <w,h,d,[ppi]>\n", arg0);
   fprintf(stderr, "                  [-raw_out]\n");
   fprintf(stderr, "                  [-YCbCr H0,V0:H1,V1:H2,V2]\n");
}
