/************************************************************************

       PACKAGE:  IMAGE ENCODER/DECODER TOOLS

       FILE:     DJPEGL.C

       AUTHORS:  Craig Watson
                 cwatson@nist.gov
                 Michael Garris
                 mgarris@nist.gov
      DATE:     11/17/2000

#cat: djpegl - Takes a Lossless JPEG (JPEGL) compressed image file and
#cat:          decodes it to an IHead or raw image file.

*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <jpegl.h>
#include <intrlv.h>
#include <ihead.h>
#include <img_io.h>
#include <ffpis/util/ioutil.h>
#include <ffpis/util/util.h>

void procargs(int, char **, char **, char **, int *, int *);

int debug = 0;

/******************/
/*Start of Program*/
/******************/

int main(int argc, char *argv[])
{
   int ret, i, n;
   char *outext;                  /* ouput file extension */
   char *ifile, ofile[MAXPATHLEN];/* file names */
   FILE *outfp;                   /* output file pointers */
   int width, height;             /* image parameters */
   int depth, ppi, ilen, olen, tlen;
   unsigned char *idata, *odata, *tdata;  /* image pointers */
   IMG_DAT *img_dat;
   int rawflag, intrlvflag;
   int lossyflag;                 /* data loss flag */
   NISTCOM *nistcom;              /* NIST Comment */
   int force_raw;


   procargs(argc, argv, &outext, &ifile, &rawflag, &intrlvflag);

   ret = read_raw_from_filesize(ifile, &idata, &ilen);
   if(ret)
      exit(ret);

   ret = jpegl_decode_mem(&img_dat, &lossyflag, idata, ilen);
   if(ret){
      free(idata);
      exit(ret);
   }

   ret = get_IMG_DAT_image(&odata, &olen, &width, &height, &depth, &ppi, img_dat);
   if(ret){
      free_IMG_DAT(img_dat, FREE_IMAGE);
      exit(ret);
   }

   if((img_dat->n_cmpnts > 1) && (intrlvflag)){
      ret = not2intrlv_mem(&tdata, &tlen, odata, img_dat->max_width,
		      img_dat->max_height, img_dat->pix_depth, img_dat->hor_sampfctr,
		      img_dat->vrt_sampfctr, img_dat->n_cmpnts);
      if(ret){
         free_IMG_DAT(img_dat, FREE_IMAGE);
         free(odata);
         return(ret);
      }
      free(odata);
      odata = tdata;
      olen = tlen;
   }
   else
      /* Forces depth 8 image to default non-interleaved, makes more sense. */
      intrlvflag = 0;

   if(debug > 1)
      fprintf(stdout, "Image pixmap constructed\n");

   fileroot(ifile);
   sprintf(ofile, "%s.%s", ifile, NCM_EXT);

   /* Get NISTCOM from compressed data file */
   ret = getc_nistcom_jpegl(&nistcom, idata, ilen);
   if(ret){
      free_IMG_DAT(img_dat, FREE_IMAGE);
      free(idata);
      exit(ret);
   }
   free(idata);

   /* Combine NISTCOM with image features */
   ret = combine_jpegl_nistcom(&nistcom, width, height, depth, ppi, lossyflag,
		   img_dat->n_cmpnts, img_dat->hor_sampfctr, img_dat->vrt_sampfctr,
		   intrlvflag, img_dat->predict[0]);
   if(ret){
      free_IMG_DAT(img_dat, FREE_IMAGE);
      free(odata);
      freefet(nistcom);
      exit(ret);
   }

   /* Remove JPEGL compression attributes from NISTCOM. */
   ret = del_jpegl_nistcom(nistcom);
   if(ret){
      free_IMG_DAT(img_dat, FREE_IMAGE);
      free(odata);
      freefet(nistcom);
      exit(ret);
   }

   force_raw = 0;
   if(img_dat->n_cmpnts > 1){
      if(depth != 24){
         if(!rawflag) {
            fprintf(stderr, "WARNING : main : ");
            fprintf(stderr, "image pixel depth = %d != 24 : ", depth);
            fprintf(stderr, "forcing raw output\n");
         }
         force_raw = 1;
      }
      else{
         for(i = 0; i < img_dat->n_cmpnts; i++){
            if((img_dat->hor_sampfctr[i] > 1) ||
                    (img_dat->vrt_sampfctr[i] > 1)){
               if(!rawflag) {
                  fprintf(stderr, "WARNING : main : ");
                  fprintf(stderr, "image results subsampled : ");
                  fprintf(stderr, "forcing raw output\n");
               }
               force_raw = 1;
               break;
            }
         }
      }
   }

   free_IMG_DAT(img_dat, FREE_IMAGE);

   /* Write NISTCOM */
   ret = writefetfile_ret(ofile, nistcom);
   if(ret){
      free(odata);
      freefet(nistcom);
      exit(ret);
   }
   freefet(nistcom);

   /* Write decoded image file. */
   sprintf(ofile, "%s.%s", ifile, outext);
   if(force_raw){
      if((outfp = fopen(ofile, "wb")) == NULL) {
         fprintf(stderr, "ERROR: main : fopen : %s\n",ofile);
         free(odata);
         exit(-2);
      }

      if((n = fwrite(odata, 1, olen, outfp)) != olen){
         fprintf(stderr, "ERROR: main : fwrite : ");
         fprintf(stderr, "only %d of %d bytes written from file %s\n",
                 n, olen, ofile);
         free(odata);
         exit(-3);
      }
      fclose(outfp);
   }
   else {
	   ret = write_raw_or_ihead(!rawflag, ofile, odata, width, height, depth, ppi);
	   if(ret){
		   free(odata);
		   exit(ret);
	   }
   }
   free(odata);

   if(debug > 1)
      fprintf(stdout, "Image pixmap written to %s\n", ofile);

   exit(0);
}

/*****************************************************************/
void print_usage(const char *arg0)
{
   fprintf(stderr, "Usage: %s <outext> <image file>\n", arg0);
   fprintf(stderr, "               [-raw_out [-nonintrlv]]\n");
}

/*****************************************************************/
void procargs(int argc, char **argv, char **outext, char **ifile,
              int *rawflag, int *intrlvflag)
{
   int argi;

   if((argc < 3) || (argc > 5)){
      print_usage(argv[0]);
      exit(-1);
   }

   *outext = argv[1];
   *ifile = argv[2];
   *rawflag = 0;
   *intrlvflag = 1;

   if(argc == 3)
      return;

   argi = 3;
   /* If rawflag ... */
   if(strncmp(argv[argi], "-r", 2) == 0){
      *rawflag = 1;
      argi++;
      if(argi >= argc)
         return;
   }
   else{
      print_usage(argv[0]);
      fprintf(stderr, "      expected option \"-raw_out\"\n");
      exit(-1);
   }
      
   /* If nonintrlv flag ... */
   if(strncmp(argv[argi], "-n", 2) == 0){
      *intrlvflag = 0;
      argi++;
   }

   if(argi < argc){
      print_usage(argv[0]);
      exit(-1);
   }
}

