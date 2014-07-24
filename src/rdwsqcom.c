/************************************************************************

      PACKAGE:  IMAGE ENCODER/DECODER TOOLS

      FILE:     RDWSQCOM.C

      AUTHORS:  Craig Watson
                cwatson@nist.gov
      DATE:     02/02/2001

#cat: rdwsqcom - Takes a WSQ compressed image file and prints all
#cat:            comments in the file to standard output.

*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>
#include <wsq.h>
#include <img_io.h>
#include <imgtype.h>
#include <ffpis/util/util.h>

void procargs(int, char **, char **);

int debug = 0; 

/******************/
/*Start of Program*/
/******************/

int main(int argc, char *argv[])
{
   int ret, img_type;
   char *ifile;
   unsigned char *idata;
   int ilen;

   procargs(argc, argv, &ifile);

   ret = read_raw_from_filesize(ifile, &idata, &ilen);
   if(ret)
      exit(ret);

   ret = image_type(&img_type, idata, ilen);
   if(ret) {
      free(idata);
      exit(ret);
   }

   if(img_type != WSQ_IMG) {
      fprintf(stderr, "ERROR : main : image is not WSQ compressed\n");
      free(idata);
      exit(-1);
   }

   ret = print_comments_wsq(stdout, idata, ilen);
   if(ret) {
      free(idata);
      exit(ret);
   }
   free(idata);

   exit(0);
}

/*****************************************************************/
void procargs(int argc, char *argv[], char **ifile)
{
   if(argc != 2){
      fprintf(stderr, "Usage: %s <image file>\n", argv[0]);
      exit(-1);
   }

   *ifile = argv[1];
   return;
}

void print_usage(const char *s)
{
      fprintf(stderr, "Usage: %s <image file>\n", s);
      exit(-1);
}
