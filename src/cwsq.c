/************************************************************************

      PACKAGE:  IMAGE ENCODER/DECODER TOOLS

      FILE:     CWSQ.C

      AUTHORS:  Craig Watson
                cwatson@nist.gov
                Michael Garris
                mgarris@nist.gov
      DATE:     11/24/1999

#cat: cwsq - Takes an IHead or raw image pixmap and encodes it using WSQ.
#cat:        This is an implementation based on the Crinimal Justice
#cat:        Information Services (CJIS) document "WSQ Gray-scale
#cat:        Fingerprint Compression Specification", Dec. 1997.

*************************************************************************/

/*****************/
/* Include files */
/*****************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <wsq.h>
#include <ihead.h>
#include <img_io.h>
#include <parsargs.h>
#include <getnset.h>
#include <ffpis/util/dataio.h>
#include <ffpis/util/ioutil.h>

void procargs(int, char **, float *, char **, char **, int *,
              int *, int *, int *, int *, char **);
void print_usage(char *);

/* Contols globally, the level of debug reporting */
/* in this application. */
int debug = 0;

/******************/
/*Start of Program*/
/******************/

int main(int argc, char *argv[])
{
   int ret;
   char *outext;
   int rawflag;             /* input image flag: 0 == Raw, 1 == IHead */
   float r_bitrate;         /* target bit compression rate */
   char *ifile, *cfile, ofile[MAXPATHLEN];     /* Input/Output filenames */
   IHEAD *ihead;            /* Ihead pointer */
   unsigned char *idata;    /* Input data */
   int width, height;       /* image characteristic parameters */
   int depth, ppi;
   unsigned char *odata;    /* Output data */
   int olen;                /* Number of bytes in output data. */
   char *comment_text;


   /* Process the command-line argument list. */
   procargs(argc, argv, &r_bitrate, &outext, &ifile, &rawflag,
            &width, &height, &depth, &ppi, &cfile);

   /* Read the image into memory (IHead or raw pixmap). */
   ret = read_raw_or_ihead_wsq(!rawflag, ifile,
		   &ihead, &idata, &width, &height, &depth);
   if(ret)
      exit(ret);

   if(debug > 0)
      fprintf(stdout, "File %s read\n", ifile);

   /* If IHead image file ... */
   if(!rawflag){
      /* Get PPI from IHead. */
      ppi = get_density(ihead);
      free(ihead);
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

   /* Encode/compress the image pixmap. */
   ret = wsq_encode_mem(&odata, &olen, r_bitrate,
		   idata, width, height, depth, ppi, comment_text);
   if(ret){
      free(idata);
      if(comment_text != (char *)NULL)
         free(comment_text);
      exit(ret);
   }

   free(idata);
   if(comment_text != (char *)NULL)
      free(comment_text);

   if(debug > 0)
      fprintf(stdout, "Image data encoded, compressed byte length = %d\n",
              olen);

   /* Generate the output filename. */
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

   /* Exit normally. */
   exit(0);
}

/*****************************************************************/
void procargs(int argc, char **argv, float *r_bitrate,
              char **outext, char **ifile, int *rawflag,
              int *width, int *height, int *depth, int *ppi, char **cfile)
{
   if((argc < 4) || (argc > 7)){
      print_usage(argv[0]);
      exit(-1);
   }

   sscanf(argv[1], "%f", r_bitrate);
   *outext = argv[2];
   *ifile = argv[3];
   *rawflag = 0;
   *width = -1;
   *height = -1;
   *depth = -1;
   *ppi = -1;
   *cfile = (char *)NULL;

   /* If argc == 4, we are done here. */

   /* If IHead image file ... */
   if(argc == 5){
      *cfile = argv[4];
   }
   /* Otherwise Raw image file ... */
   else if(argc == 6 || argc == 7){
      /* If rawflag ... */
      if(strncmp(argv[4], "-r", 2) == 0){
         *rawflag = 1;
         parse_w_h_d_ppi(argv[5], argv[0], width, height, depth, ppi);
         if(*depth != 8){
            print_usage(argv[0]);
            fprintf(stderr, "       image depth = %d not 8\n", *depth);
            exit(-1);
         }
      }
      else{
         print_usage(argv[0]);
         fprintf(stderr,
                 "       invalid arg 4, expected \"-raw_in\" option\n");
         exit(-1);
      }
      /* If Comment file ... */
      if(argc == 7){
         *cfile = argv[6];
      }
   }
}

/*****************************************************************/
void print_usage(char *arg0)
{
   fprintf(stderr, "Usage: %s ", arg0);
   fprintf(stderr, "<r bitrate> <outext> <image file>\n");
   fprintf(stderr,
           "                 [-raw_in w,h,d,[ppi]] [comment file]\n\n");
   fprintf(stderr,
           "   r bitrate = compression bit rate (2.25==>5:1, .75==>15:1)\n\n");
}
