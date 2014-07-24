/************************************************************************

      PACKAGE:  IMAGE ENCODER/DECODER TOOLS

      FILE:     DWSQ.C

      AUTHORS:  Craig Watson
                cwatson@nist.gov
                Michael Garris
                mgarris@nist.gov
      DATE:     11/24/1999

#cat: dwsq - Takes a WSQ compressed image file and decodes it,
#cat:        reconstructing a "lossy" pixmap and saving it to
#cat:        an IHead or raw image file.
#cat:        This is an implementation based on the Crinimal Justice
#cat:        Information Services (CJIS) document "WSQ Gray-scale
#cat:        Fingerprint Compression Specification", Dec. 1997.

*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <wsq.h>
#include <ihead.h>
#include <ffpis/util/ioutil.h>
#include <img_io.h>
#include <ffpis/util/util.h>

void procargs(int , char **, char **, char **, int *);

int debug = 0; 

/******************/
/*Start of Program*/
/******************/

int main( int argc, char **argv)
{
   int ret;
   int rawflag;                   /* raw input data or Ihead image */
   char *outext;                  /* ouput file extension */
   char *ifile, ofile[MAXPATHLEN];  /* file names */
   int ilen;
   int width, height;             /* image parameters */
   int depth, ppi;
   unsigned char *idata, *odata;  /* image pointers */
   int lossyflag;                 /* data loss flag */
   NISTCOM *nistcom;              /* NIST Comment */
   char *ppi_str;


   procargs(argc, argv, &outext, &ifile, &rawflag);

   ret = read_raw_from_filesize(ifile, &idata, &ilen);
   if(ret)
      exit(ret);

   ret = wsq_decode_mem(&odata, &width, &height, &depth, &ppi, &lossyflag, idata, ilen);
   if(ret){
      free(idata);
      exit(ret);
   }

   if(debug > 1)
      fprintf(stderr, "Image pixmap constructed\n");

   fileroot(ifile);
   sprintf(ofile, "%s.%s", ifile, NCM_EXT);

   /* Get NISTCOM from compressed data file */
   ret = getc_nistcom_wsq(&nistcom, idata, ilen);
   if(ret){
      free(idata);
      exit(ret);
   }
   free(idata);
   /* WSQ decoder always returns ppi=-1, so believe PPI in NISTCOM, */
   /* if it already exists. */
   ppi_str = (char *)NULL;
   if(nistcom != (NISTCOM *)NULL){
      ret = extractfet_ret(&ppi_str, NCM_PPI, nistcom);
      if(ret){
         free(odata);
         freefet(nistcom);
         exit(ret);
      }
   }
   if(ppi_str != (char *)NULL){
      ppi = atoi(ppi_str);
      free(ppi_str);
   }

   /* Combine NISTCOM with image features */
   ret = combine_wsq_nistcom(&nistcom, width, height, depth, ppi, lossyflag,
		   0.0 /* will be deleted next */);
   if(ret){
     free(odata);
     if(nistcom != (NISTCOM *)NULL)
        freefet(nistcom);
     exit(ret);
   }
   ret = del_wsq_nistcom(nistcom);
   if(ret){
     free(odata);
     freefet(nistcom);
     exit(ret);
   }

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
   ret = write_raw_or_ihead(!rawflag, ofile, odata, width, height, depth, ppi);
   if(ret){
      free(odata);
      exit(ret);
   }
   free(odata);

   if(debug > 1)
      fprintf(stdout, "Image pixmap written to %s\n", ofile);

   exit(0);
}

/*****************************************************************/
void procargs(int argc, char **argv, char **outext, char **ifile, int *rawflag)
{
   if((argc < 3) || (argc > 4)){
      fprintf(stderr, "Usage: %s <outext> <image file> [-raw_out]\n", argv[0]);
      exit(-1);
   }

   *rawflag = 0;
   *outext = argv[1];
   *ifile = argv[2];

   if(argc == 4){
      /* If rawflag ... */
      if(strncmp(argv[3], "-r", 2) == 0)
         *rawflag = 1;
      else{
         fprintf(stderr,
                 "Usage: %s <outext> <image file> [-raw_out]\n", argv[0]);
         fprintf(stderr,
                 "       invalid arg 3, expected \"-raw_out\" option\n");
         exit(-1);
      }
   }

   if(debug > 0)
      fprintf(stdout, "file-> %s\n", *ifile);
}

void print_usage(const char *prog)
{
	const char utxt[]="Usage: %s <outext> <image file> [-raw_out]\n";
	fprintf(stderr,utxt,prog);
	exit(-1);
}
