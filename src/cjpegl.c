/************************************************************************

      PACKAGE:  IMAGE ENCODER/DECODER TOOLS

      FILE:     CJPEGL.C

      AUTHORS:  Craig Watson
                cwatson@nist.gov
                Michael Garris
                mgarris@nist.gov
      DATE:     11/17/2000

#cat: cjpegl - Takes an IHead or raw image pixmap and encodes it using
#cat:        Lossless JPEG (JPEGL), writing the compressed data to file.

*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <math.h>
#include <jpegl.h>
#include <intrlv.h>
#include <ihead.h>
#include <img_io.h>
#include <parsargs.h>
#include <getnset.h>
#include <dataio.h>
#include <ffpis/util/ioutil.h>

void procargs(int, char **, char **, char **, int *, int *, int *, int *,
              int *, int *, int *, int *, int *, char **);
void print_usage(char *);

int debug = 0;

/******************/
/*Start of Program*/
/******************/

int main(int argc, char *argv[])
{
   int ret, rawflag, intrlvflag;
   char *outext;                  /* ouput file extension */
   char *ifile, *cfile, ofile[MAXPATHLEN]; /* file names */
   int width, height, depth, ppi, ilen, olen;  /* image parameters */
   IHEAD *ihead;
   unsigned char *idata, *odata;          /* image pointers */
   IMG_DAT *img_dat;
   int hor_sampfctr[MAX_CMPNTS], vrt_sampfctr[MAX_CMPNTS];
   int n_cmpnts;
   char *comment_text;



   procargs(argc, argv, &outext, &ifile, &rawflag,
            &width, &height, &depth, &ppi, &intrlvflag,
            hor_sampfctr, vrt_sampfctr, &n_cmpnts, &cfile);

   /* If nonintrlv or H,V's are specified for YCbCr ... */
   if((!intrlvflag) || (argc > 6)){
      ret = read_raw_from_filesize(ifile, &idata, &ilen);
      if(ret)
         exit(ret);
      ret = test_image_size(ilen, width, height, hor_sampfctr, vrt_sampfctr,
		      n_cmpnts, intrlvflag);
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

      /* If IHead file, then n_cmpnts must be set based on pixel depth. */
      if(depth == 8)
         n_cmpnts = 1;
      else if(depth == 24)
         n_cmpnts = 3;
      else{
         fprintf(stderr, "ERROR : main : depth = %d not equal to 8 or 24\n",
                 depth);
         exit(-2);
      }
   }

   if(cfile == (char *)NULL)
      comment_text = (char *)NULL;
   else{
      ret = read_ascii_file(cfile, &comment_text);
      if(ret){
         free(idata);
         exit(ret);
      }
   }

   /* If necessary, convert to non-interleaved. */
   if((n_cmpnts > 1) && (intrlvflag)){
      ret = intrlv2not_mem(&odata, &olen, idata, width, height, depth, hor_sampfctr,
		      vrt_sampfctr, n_cmpnts);
      if(ret){
         free(idata);
         if(comment_text != (char *)NULL)
            free(comment_text);
         exit(ret);
      }
      free(idata);
      idata = odata;
      ilen = olen;
   }

   ret = setup_IMG_DAT_nonintrlv_encode(&img_dat, idata, width, height, depth,
		   ppi, hor_sampfctr, vrt_sampfctr, n_cmpnts, 0, PRED4);
   if(ret){
      free(idata);
      if(comment_text != (char *)NULL)
         free(comment_text);
      return(ret);
   }
   free(idata);

   if(debug > 0){
      fprintf(stdout, "Image structure initialized\n");
      fflush(stderr);
   }

   ret = jpegl_encode_mem(&odata, &olen, img_dat, comment_text);
   if(ret){
      free_IMG_DAT(img_dat, FREE_IMAGE);
      if(comment_text != (char *)NULL)
         free(comment_text);
      exit(ret);
   }

   free_IMG_DAT(img_dat, FREE_IMAGE);
   if(comment_text != (char *)NULL)
      free(comment_text);

   if(debug > 0)
      fprintf(stdout, "Image data encoded, compressed byte length = %d\n",
              olen);

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
              int *width, int *height, int *depth, int *ppi, int *intrlvflag,
              int *hor_sampfctr, int *vrt_sampfctr, int *n_cmpnts,
              char **cfile)
{
   int argi;

   if((argc < 3) || (argc > 9)){
      print_usage(argv[0]);
      exit(-1);
   }

   *outext = argv[1];
   *ifile = argv[2];
   *rawflag = 0;
   *intrlvflag = 1;
   *width = -1;
   *height = -1;
   *depth = -1;
   *ppi = -1;
   *cfile = (char *)NULL;
   *n_cmpnts = 0;

   hor_sampfctr[0] = 1;
   vrt_sampfctr[0] = 1;
   hor_sampfctr[1] = 1;
   vrt_sampfctr[1] = 1;
   hor_sampfctr[2] = 1;
   vrt_sampfctr[2] = 1;

   /* If argc == 3, we are done here. */

   /* If IHead image file ... */
   if(argc == 4){
      if((strncmp(argv[3], "-r", 2) == 0) || (strncmp(argv[3], "-n", 2) == 0)
         || (strncmp(argv[3], "-Y", 2) == 0)){
            print_usage(argv[0]);
            fprintf(stderr, "       invalid arg 4 expecting comment file not\n");
            fprintf(stderr, "       -raw_in, -noninrlv, or -YCbCr\n");
            exit(-1);
      }
      *cfile = argv[3];
   }
   /* Otherwise Raw image file ... */
   else if(argc >= 5){
      /* If rawflag ... */
      if(strncmp(argv[3], "-r", 2) == 0){
         *rawflag = 1;
         parse_w_h_d_ppi(argv[4], argv[0], width, height, depth, ppi);
         if(*depth == 8)
            *n_cmpnts = 1;
         else if(*depth == 24)
            *n_cmpnts = 3;
         else{
            print_usage(argv[0]);
            fprintf(stderr, "       depth = %d not equal to 8 or 24\n",
                    *depth);
            exit(-1);
         }
      }
      else{
         print_usage(argv[0]);
         fprintf(stderr, "       invalid arg 4, expected \"-raw\" option\n");
         exit(-1);
      }

      argi = 5;
      if(argi >= argc)
         return;
      /* If non-intrlvflag ... */
      if(strncmp(argv[argi], "-n", 2) == 0){
         if(*depth == 8){
            print_usage(argv[0]);
            fprintf(stderr, "       invalid -nonintrlv option, depth = 8\n");
            exit(-1);
         }
         *intrlvflag = 0;
         argi++;
         if(argi >= argc)
            return;
      }
      /* If YCbCr flag ... */
      if(strncmp(argv[argi], "-Y", 2) == 0){
         if(*depth == 8){
            print_usage(argv[0]);
            fprintf(stderr, "       invalid -YCbCr option, depth = 8\n");
            exit(-1);
         }
         argi++;
         if(argi >= argc){
            print_usage(argv[0]);
            fprintf(stderr, "       -YCbCr flag missing sample factors\n");
            exit(-1);
         }
         parse_h_v_sampfctrs(argv[argi], argv[0],
                             hor_sampfctr, vrt_sampfctr, n_cmpnts);
         argi++;
      }

      /* If Comment file ... */
      if(argi < argc){
         *cfile = argv[argi];
         argi++;
      }

      if(argi != argc){
         print_usage(argv[0]);
         fprintf(stderr, "       too many arguments specified\n");
         exit(-1);
      }
   }
}

/*****************************************************************/
void print_usage(char *arg0)
{
   fprintf(stderr, "Usage: %s <outext> <image file>\n", arg0);
   fprintf(stderr, "                  [-raw_in w,h,d,[ppi]\n");
   fprintf(stderr, "                     [-nonintrlv]\n");
   fprintf(stderr, "                     [-YCbCr H0,V0:H1,V1:H2,V2]]\n");
   fprintf(stderr, "                  [comment file]\n");
}
