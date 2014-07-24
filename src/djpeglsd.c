/************************************************************************

      PACKAGE:  IMAGE ENCODER/DECODER TOOLS

      FILE:     DJPEGL.C

      AUTHORS:  Craig Watson
                cwatson@nist.gov
      DATE:     12/15/2000

#cat: djpeglsd - Takes an IHead formatted, JPEGL compressed, image file,
#cat:            such as those distributed with NIST Special Databases
#cat:            {4,9,10,18} and decodes it, reconstructing a "lossless"
#cat:            pixmap and saving it to an IHead or raw image file.
#cat:            If the input file is from a NIST Special Database, then
#cat:            image attributes are stored as a separate NISTCOM text file.
#cat:            This program should be used to convert legacy data only.
#cat:            The format of the files processed by this program should
#cat:            be considered obsolete.

*************************************************************************/

#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <stdlib.h>
#include <jpeglsd4.h>
#include <ihead.h>
#include <img_io.h>
#include <nistcom.h>
#include <ffpis/util/util.h>
#include <ffpis/util/ioutil.h>
#include <ffpis/util/memalloc.h>

#define SD_NUM 4
int sd_list[] = {4,9,10,18};


/**************/
/*routine list*/
/**************/

void procargs(int, char **, char **, char **, int *, int *);

int debug = 0;
/**************/
/*main routine*/
/**************/

int main(int argc, char *argv[])
{
   FILE *fp;
   int ret, rawflag, sd_id;
   char *outext, *ifile, ofile[MAXPATHLEN];
   IHEAD *ihead;
   int width, height, depth, ppi;
   unsigned char *idata, *odata;
   int i, complen, compcode;
   NISTCOM *nistcom;


   procargs(argc, argv, &outext, &ifile, &sd_id, &rawflag);
   nistcom = (NISTCOM *)NULL;

   /* Read ihead header to get image compression information */
   fp = fopen(ifile, "rb");
   if(fp == (FILE *)NULL) {
      fprintf(stderr, "ERROR : main : fopen : %s\n", ifile);
      exit(-1);
   }

   ihead = readihdr(fp);
   if(ihead == (IHEAD *)NULL) {
      fprintf(stderr, "ERROR : main : readihdr :  ihead\n");
      fclose(fp);
      exit(-1);
   }

   /* Get image attributes from header */
   sscanf(ihead->height,"%d", &height);
   sscanf(ihead->width,"%d", &width);
   sscanf(ihead->depth,"%d", &depth);
   sscanf(ihead->density,"%d", &ppi);
   sscanf(ihead->complen,"%d", &complen);
   sscanf(ihead->compress,"%d", &compcode);

   /* Convert image attributes to a nistcom comment */
   if(sd_id){
      ret = sd_ihead_to_nistcom(&nistcom, ihead, sd_id);
      if(ret) {
         fclose(fp);
         free(ihead);
         exit(ret);
      }
   }
   free(ihead);

   ret = combine_jpegl_nistcom(&nistcom, width, height, depth, ppi,
		   0, 1, (int *)NULL, (int *)NULL, 0, -1);
   if(ret) {
      fclose(fp);
      exit(ret);
   }
   ret = del_jpegl_nistcom(nistcom);
   if(ret){
      freefet(nistcom);
      fclose(fp);
      exit(ret);
   }

   /* If old JPEGL compressed file ... */
   if(compcode == JPEG_SD) {
      /* Allocate space and read compressed data */
      idata = (unsigned char *)malloc(complen * sizeof(unsigned char));
      if(idata == (unsigned char *)NULL) {
         fprintf(stderr, "ERROR : main : malloc : idata\n");
         fclose(fp);
         freefet(nistcom);
         exit(-1);
      }
      i = fread(idata, sizeof(unsigned char), complen, fp);
      if(i != complen) {
         fprintf(stderr, "ERROR : main : fread : %s\n", ifile);
         fclose(fp);
         freefet(nistcom);
         free(idata);
         exit(-1);
      }
      fclose(fp);

      /* Allocate space for decompressed data */
      malloc_uchar(&odata, width*height, "main");
      odata = (unsigned char *)malloc(width*height * sizeof(unsigned char));
      if(odata == (unsigned char *)NULL) {
         fprintf(stderr, "ERROR : main : malloc : odata\n");
         freefet(nistcom);
         free(idata);
         exit(-1);
      }
   
      /* Decompress data block */
      ret = jpegl_sd4_decode_mem(idata, complen, width, height, depth, odata);
      if(ret){
         freefet(nistcom);
         free(idata);
         free(odata);
         exit(ret);
      }
      free(idata);

      fileroot(ifile);
      sprintf(ofile, "%s.%s", ifile, NCM_EXT);

      /* Write a nistcom file */
      ret = writefetfile_ret(ofile, nistcom);
      if(ret){
         freefet(nistcom);
         free(odata);
         exit(ret);
      }
      freefet(nistcom);

      sprintf(ofile, "%s.%s", ifile, outext);

      /* Write decompressed data */
      ret = write_raw_or_ihead(!rawflag, ofile, odata, width, height, depth, ppi);
      if(ret) {
         free(odata);
         exit(ret);
      }
      free(odata);
   }
   /* else, Not a old JPEGL compressed file ... */
   else
      fprintf(stderr, "WARNING : main : Image not JPEGL SD[4,9,10,18] compressed, DO NOTHING\n");

   exit(0);
}


/*******************************************/
/*routine to process command line arguments*/
/*******************************************/
void procargs(int argc, char *argv[], char **outext, char **ifile,
              int *sd_id, int *rawflag)
{
   int i, argi, found;

   if(argc < 3) {
      fprintf(stderr, "Usage: %s <outext> <image file> [-sd #] [-raw_out]\n", argv[0]);
      exit(-1);
   }

   *outext = argv[1];
   *ifile = argv[2];
   *rawflag = 0;
   *sd_id = 0;

   if(argc == 3)
      return;

   argi = 3;
   if(strncmp(argv[argi], "-s", 2) == 0) {
      argi++;
      if(argc >= 5) {
         *sd_id = atoi(argv[argi]);

         found = 0;
         for(i = 0; i < SD_NUM; i++){
            if(*sd_id == sd_list[i]){
               found = 1;
               break;
            }
         }

         if(!found){
            fprintf(stderr, "Usage: %s <outext> <image file> [-sd #] [-raw_out]\n", argv[0]);
            fprintf(stderr, "              SD list = {4,9,10,18}\n");
            fprintf(stderr, "              SD %d is not a recognized database\n", *sd_id);
            exit(-1);
         }
      }
      else {
         fprintf(stderr, "Usage: %s <outext> <image file> [-sd #] [-raw_out]\n", argv[0]);
         exit(-1);
      }
      argi++;
      if(argi >= argc)
         return;
   }

   if(strncmp(argv[argi], "-r", 2) == 0) {
      *rawflag = 1;
      return;
   }
   else {
      fprintf(stderr, "Usage: %s <outext> <image file> [-sd #] [-raw_out]\n", argv[0]);
      exit(-1);
   }

}

void
print_usage(const char *prog)
{ 
	const char utxt[]=
	"Usage: %s <outext> <image file> [-sd #] [-raw_out]\n"
	"              SD list = {4,9,10,18}\n" ;
	fprintf(stderr,utxt,prog);
	exit(-1);
}

