/************************************************************************

      PACKAGE:  IMAGE ENCODER/DECODER TOOLS

      FILE:     YCC2RGB.C

      AUTHORS:  Craig Watson
                cwatson@nist.gov
                Michael Garris
                mgarris@nist.gov
      DATE:     02/15/2001

#cat: ycc2rgb - Takes a raw YCbCr pixmap and converts its pixels components
#cat:           to the RGB colorspace.  This program can handle the specific
#cat:           downsampling of comonent planes.

*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/param.h>
#include <intrlv.h>
#include <ihead.h>
#include <img_io.h>
#include <rgb_ycc.h>
#include <parsargs.h>
#include <ffpis/util/util.h>
#include <ffpis/util/ioutil.h>

void procargs(int, char **, char **, char **, int *, int *, int *, int *,
              int *, int *, int *, int *, int *);

int debug = 0;

/******************/
/*Start of Program*/
/******************/

int main(int argc, char *argv[])
{
   int ret, n, rawflag;
   char *outext;                   /* ouput file extension */
   char *ifile, ofile[MAXPATHLEN]; /* file names */
   int width, height, depth, ppi;  /* image parameters */
   int ilen, olen, tlen;
   int intrlvflag;
   unsigned char *idata, *odata, *tdata; /* image pointers */
   int hor_sampfctr[MAX_CMPNTS], vrt_sampfctr[MAX_CMPNTS];
   int out_hor_sampfctr[MAX_CMPNTS], out_vrt_sampfctr[MAX_CMPNTS];
   int n_cmpnts;

   procargs(argc, argv, &outext, &ifile,
            &width, &height, &depth, &ppi, &rawflag, &intrlvflag,
            hor_sampfctr, vrt_sampfctr, &n_cmpnts);

   if(!test_evenmult_sampfctrs(&n, &n, hor_sampfctr, vrt_sampfctr, n_cmpnts)){
      fprintf(stderr, "ERROR : main : ");
      fprintf(stderr, "sample factors must be even multiples\n");
      exit(-1);
   }

   ret = read_raw_from_filesize(ifile, &idata, &ilen);
   if(ret)
      exit(ret);

   ret = test_image_size(ilen, width, height, hor_sampfctr,
		   vrt_sampfctr, n_cmpnts, intrlvflag);
   if(ret)
      exit(ret);

   if(debug > 0)
      fprintf(stdout, "File %s read\n", ifile);

   if(intrlvflag){
      ret = intrlv2not_mem(&tdata, &tlen, idata, width, height, depth,
		      hor_sampfctr, vrt_sampfctr, n_cmpnts);
      if(ret){
         free(idata);
         exit(ret);
      }
      free(idata);
      idata = tdata;
      ilen = tlen;
   }

   ret = upsample_cmpnts(&tdata, &tlen, idata, width, height, depth,
		   hor_sampfctr, vrt_sampfctr, n_cmpnts);
   if(ret){
      free(idata);
      exit(ret);
   }
   free(idata);
   idata = tdata;
   ilen = tlen;

   if(debug > 0)
      fprintf(stdout, "YCbCr data upsampled\n");

   ret = ycc2rgb_nonintrlv_mem(&odata, &olen, idata, width, height, depth);
   if(ret){
      free(idata);
      exit(ret);
   }
   free(idata);

   if(debug > 0)
      fprintf(stdout, "Image data converted to YCbCr\n");

   if(intrlvflag){
      out_hor_sampfctr[0] = 1;
      out_vrt_sampfctr[0] = 1;
      out_hor_sampfctr[1] = 1;
      out_vrt_sampfctr[1] = 1;
      out_hor_sampfctr[2] = 1;
      out_vrt_sampfctr[2] = 1;
      ret = not2intrlv_mem(&tdata, &tlen, odata, width, height, depth,
		      out_hor_sampfctr, out_vrt_sampfctr, n_cmpnts);
      if(ret){
         free(odata);
         exit(ret);
      }
      free(odata);
      odata = tdata;
      olen = tlen;
   }

   fileroot(ifile);
   sprintf(ofile, "%s.%s", ifile, outext);

   ret = write_raw_or_ihead(!rawflag, ofile, odata, width, height, depth, ppi);
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
              char **outext, char **ifile,
              int *width, int *height, int *depth, int *ppi,
              int *rawflag, int *intrlvflag,
              int *hor_sampfctr, int *vrt_sampfctr, int *n_cmpnts)
{
   int argi, n_cmpnts_tst;

   if((argc < 4) || (argc > 8)){
      print_usage(argv[0]);
      exit(-1);
   }

   *outext = argv[1];
   *ifile = argv[2];
   *width = -1;
   *height = -1;
   *depth = -1;
   *ppi = -1;
   *rawflag = 0;
   *intrlvflag = 1;
   hor_sampfctr[0] = 1;
   vrt_sampfctr[0] = 1;
   hor_sampfctr[1] = 1;
   vrt_sampfctr[1] = 1;
   hor_sampfctr[2] = 1;
   vrt_sampfctr[2] = 1;
   *n_cmpnts = 3;

   argi = 3;
   parse_w_h_d_ppi(argv[argi], argv[0], width, height, depth, ppi);
   if(*depth != 24){
      print_usage(argv[0]);
      fprintf(stderr, "       depth = %d not equal to 24\n",
              *depth);
      exit(-1);
   }
   argi++;
   if(argi == argc)
      return;

   /* If rawflag ... */
   if(strncmp(argv[argi], "-r", 2) == 0){
      *rawflag = 1;
      argi++;
      if(argi == argc)
         return;
   }

   /* If nonintrlv flag ... */
   if(strncmp(argv[argi], "-n", 2) == 0){
      *intrlvflag = 0;
      /* rawflag must be set */
      if(*rawflag == 0){
         print_usage(argv[0]);
         fprintf(stderr, "       -raw_out must be used with -nonintrlv\n");
         exit(-1);
      }
      argi++;
      if(argi == argc)
         return;
   }

   /* If YCbCr flag ... */
   if(strncmp(argv[argi], "-Y", 2) == 0){
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
void print_usage(const char *arg0)
{
   fprintf(stderr, "Usage: %s <outext> <image file> <w,h,d,[ppi]>\n", arg0);
   fprintf(stderr, "                  [-raw_out]\n");
   fprintf(stderr, "                  [-nonintrlv]\n");
   fprintf(stderr, "                  [-YCbCr H0,V0:H1,V1:H2,V2]\n");
}
