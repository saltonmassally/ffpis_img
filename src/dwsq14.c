/************************************************************************

      PACKAGE:  IMAGE ENCODER/DECODER TOOLS

      FILE:     DWSQ14.C

      AUTHORS:  Craig Watson
                cwatson@nist.gov
                Michael Garris
                mgarris@nist.gov
      DATE:     11/24/1999

#cat: dwsq14 - Takes an IHead formatted, WSQ compressed, image file,
#cat:        such as those used in the distribution of legacy data on
#cat:        NIST Special Database 14 and decodes it, reconstructing a
#cat:        "lossy" pixmap and saving it to an IHead or raw image file.
#cat:        This program should be used to convert legacy data only.
#cat:        The format of the files processed by this program should
#cat:        be considered obsolete.

*************************************************************************/

#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <stdlib.h>
#include <wsq.h>
#include <ihead.h>
#include <nistcom.h>
#include <imgtype.h>
#include <img_io.h>
#include <ffpis/util/util.h>
#include <ffpis/util/ioutil.h>

void procargs(int, char **, char **, char **, int *);

int debug = 0;

/******************/
/*Start of Program*/
/******************/

int main(int argc, char *argv[])
{
   int ret, rawflag, img_type;
   unsigned char *idata, *odata;
   int ilen, width, height, depth, ppi, lossyflag;
   IHEAD *ihead;
   FILE *fp;
   char *outext, *ifile, ofile[MAXPATHLEN];
   NISTCOM *nistcom;


   procargs(argc, argv, &outext, &ifile, &rawflag);
   /* Set ppi to unknown */
   ppi = -1;
   nistcom = (NISTCOM *)NULL;

   /* Check image type */
   ret = read_raw_from_filesize(ifile, &idata, &ilen);
   if(ret)
      exit(ret);

   ret = image_type(&img_type, idata, ilen);
   if(ret) {
      free(idata);
      exit(ret);
   }
   free(idata);

   /* Open image file for reading based on image type */
   if((fp = fopen(ifile,"rb")) == NULL) {
      fprintf(stderr, "ERROR: main : fopen : %s\n",ifile);
      exit(-1);
   }
   /* If img_type is ihead ... */
   if(img_type == IHEAD_IMG){
      /* Read ihead header and check for WSQ_SD14 compression */
      ihead = readihdr(fp);
      if(atoi(ihead->compress) != WSQ_SD14){
         fprintf(stderr, "ERROR : main : input image not WSQ_SD14 compressed\n");
         fprintf(stderr, "        compression = %d, WSQ_SD14 = %d\n",
                 atoi(ihead->compress), WSQ_SD14);
         fclose(fp);
         free(ihead);
         exit(-1);
      }

      /* Get ppi from ihead header */
      sscanf(ihead->density,"%d", &ppi);

      /* Create a nistcom for the image attributes */
      ret = sd_ihead_to_nistcom(&nistcom, ihead, 14);
      if(ret) {
         fclose(fp);
         free(ihead);
         exit(ret);
      }
      free(ihead);
   }
   /* If image not WSQ_IMG or IHEAD_IMG, ERROR!!! */
   else if(img_type != WSQ_IMG) {
      fprintf(stderr, "ERROR : main : Invalid image\n");
      fprintf(stderr, "Expected a WSQ_SD14 compressed image in\n");
      fprintf(stderr, "either raw or ihead format.\n");
      fclose(fp);
      exit(-1);
   }

   /* Decode compressed image */
   ret = wsq14_decode_file(&odata, &width, &height, &depth, &lossyflag, fp);
   if(ret){
      fclose(fp);
      if(img_type == IHEAD_IMG)
         freefet(nistcom);
      exit(ret);
   }
   fclose(fp);

   if(debug > 1)
      fprintf(stderr, "Image pixmap constructed\n");

   /* Combine image attributes into current nistcom */
   ret = combine_wsq_nistcom(&nistcom, width, height, depth, ppi, lossyflag, -1.0);
   if(ret){
     if(img_type == IHEAD_IMG)
        freefet(nistcom);
     free(odata);
     exit(ret);
   }
   ret = del_wsq_nistcom(nistcom);
   if(ret){
      free(odata);
      freefet(nistcom);
      exit(ret);
   }

   fileroot(ifile);
   sprintf(ofile, "%s.%s", ifile, NCM_EXT);
   /* Write NISTCOM */
   ret = writefetfile_ret(ofile, nistcom);
   if(ret){
     freefet(nistcom);
     free(odata);
     exit(ret);
   }
   freefet(nistcom);

   /* Write decompressed image */
   sprintf(ofile, "%s.%s", ifile, outext);
   ret = write_raw_or_ihead(!rawflag, ofile, odata, width, height, depth, ppi);
   if(ret){
      free(odata);
      exit(ret);
   }
   free(odata);

   if(debug > 1)
      fprintf(stderr, "Image pixmap written to %s\n", ofile);

   exit(0);
}

/*****************************************************************/
void procargs(int argc, char *argv[], char **outext, char **ifile, int *rawflag)
{
   if(argc < 3) {
      fprintf(stderr,
      "Usage: %s <outext> <image file> [-raw_out]\n", argv[0]);
      exit(-1);
   }

   *outext = argv[1];
   *ifile = argv[2];
   *rawflag = 0;

   if(argc == 3)
      return;

   if(strncmp(argv[3], "-r", 2) == 0) {
      *rawflag = 1;
      return;
   }
   else {
      fprintf(stderr,
      "Usage: %s <output extension> <filename> [-raw_out]\n", argv[0]);
      exit(-1);
   }
}

void print_usage(const char *prog)
{
	const char utxt[]="Usage: %s <output extension> <filename> [-raw_out]\n";
	fprintf(stderr,utxt,prog);
	exit(-1);
}
