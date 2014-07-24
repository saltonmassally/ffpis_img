/************************************************************************

      PACKAGE:  IMAGE ENCODER/DECODER TOOLS

      FILE:     WRWSQCOM.C

      AUTHORS:  Craig Watson
                cwatson@nist.gov
      DATE:     02/02/2001

#cat: wrwsqcom - takes a WSQ compressed image file and inserts a
#cat:            comment in the file at the end of all other comments.

*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <wsq.h>
#include <img_io.h>
#include <imgtype.h>
#include <ffpis/util/dataio.h>
#include <ffpis/util/util.h>

void procargs(int, char **, char **, char **, char **);

int debug = 0; 

/******************/
/*Start of Program*/
/******************/

int main(int argc, char *argv[])
{
   int n, ret, img_type;
   char *ifile, *cfile, *comment_text;
   FILE *outfp;
   unsigned char *idata, *cdata;
   int ilen, clen;

   procargs(argc, argv, &ifile, &cfile, &comment_text);

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

   if(cfile != (char *)NULL) {
      ret = read_ascii_file(cfile, &comment_text);
      if(ret){
         free(idata);
         exit(ret);
      }
   }

   ret = add_comment_wsq(&cdata, &clen, idata, ilen, comment_text);
   if(ret) {
      free(idata);
      if(cfile != (char *)NULL)
         free(comment_text);
      exit(ret);
   }
   free(idata);
   if(cfile != (char *)NULL)
      free(comment_text);

   if((outfp = fopen(ifile, "wb")) == NULL) {
      fprintf(stderr, "ERROR: main : fopen : %s\n", ifile);
      free(cdata);
      exit(-2);
   }

   if((n = fwrite(cdata, 1, clen, outfp)) != clen){
      fprintf(stderr, "ERROR: main : fwrite : ");
      fprintf(stderr, "only %d of %d bytes written from file %s\n",
              n, clen, ifile);
      free(cdata);
      exit(-3);
   }
   free(cdata);
   fclose(outfp);

   exit(0);
}

/*****************************************************************/
void procargs(int argc, char *argv[], char **ifile, char **cfile,
              char **ctext)
{
   if(argc != 4){
      fprintf(stderr, "Usage: %s <image file> ", argv[0]);
      fprintf(stderr, "<-f comment file | -t comment text>\n");
      exit(-1);
   }

   *ifile = argv[1];
   *cfile = (char *)NULL;
   *ctext = (char *)NULL;

   if(strncmp(argv[2], "-f", 2) == 0)
      *cfile = argv[3];
   else if(strncmp(argv[2], "-t", 2) == 0)
      *ctext = argv[3];
   else {
      fprintf(stderr, "Usage: %s <image file> ", argv[0]);
      fprintf(stderr, "<-f comment file | -t comment text>\n");
      exit(-1);
   }

   return;
}

void print_usage(const char *s)
{
  const char estr[]="Usage: %s <image file> "
    "<-f comment file | -t comment text>\n";
  fprintf(stderr,estr,s);
  exit(-1);
}
