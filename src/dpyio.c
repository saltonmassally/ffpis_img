/***********************************************************************
      PACKAGE: NIST Image Display

      FILE:    DPYIO.C

      AUTHORS: Stan Janet
               Michael D. Garris
      DATE:    12/03/1990

      ROUTINES:
               readfile()
               createfile()
               unlinkfile()
               buildheader()
               writeheader()
               readheader()
               writedata()
               readdata()
               fdclose()

***********************************************************************/

#include <stdio.h>
#include <errno.h>
#include <values.h>
#include <sys/types.h>
#include <sys/param.h>
#include <imgtype.h>
#include <imgdecod.h>
#include <dpyimage.h>

/**********************************************************************/
int readfile(char *filename, u_char **data, int *bpi, u_int *iw, u_int *ih,
             u_int *depth, u_int *whitepix, int *align)
{
   extern int raw;
   extern u_int raw_w, raw_h, raw_depth, raw_whitepix;
   IHEAD *head;
   int ret, ilen, img_type;

   ret = read_and_decode_dpyimage(filename, &img_type, data, &ilen,
                                  (int *)iw, (int *)ih, (int *)depth, bpi);

   /* If ERROR on reading file ... */
   if(ret < 0){
      return(ret);
   }

   if(ret == IMG_IGNORE)
      return(ret);

   if(img_type == UNKNOWN_IMG){
      if(raw){
         *bpi = 0;
         *align = BITSPERBYTE;
         *whitepix = raw_whitepix;
         *iw = raw_w;
         *ih = raw_h;
         *depth = raw_depth;
      }
      else{
         fprintf(stderr, "ERROR : readfile : input file %s is assumed RAW\n",
                 filename);
         fprintf(stderr, "                   -r option must be specified\n");
         return(-2);
      }
   }
   else{
      *align = BITSPERBYTE;
      if(*depth == 1)
         *whitepix = 0;
      else
         *whitepix = 255;
   }

   return(0);
}

/***************************************************************/
int createfile(char *filename)
{
   int ret;
   FILE *fp;

   fp = fopen(filename,"wb");
   if (fp == (FILE *) NULL) {
      (void) fprintf(stderr,"dpyimage: cannot open file %s for writing\n",
                     filename);
      return(-2);
   }

   if(ret = fclose(fp)){
      (void) fprintf(stderr,"dpyimage: cannot close file %s\n",
                     filename);
      return(ret);
   }

   return(0);
}

/**************************************************************/
void unlinkfile(char *filename)
{
   if (unlink(filename) < 0) {
      char buffer[2*MAXPATHLEN];
      int e;

      e = errno;
      (void) sprintf(buffer,"dpyimage: cannot unlink %s",filename);
      errno = e;
      perror(buffer);
   }
}

/****************************************************************/
void buildheader(struct header_t *header, char *filename,
                 u_int w, u_int h, u_int depth, u_int whitepix, int align)
{
   (void) strcpy(header->filename,filename);
   header->iw = w;
   header->ih = h;
   header->depth = depth;
   header->whitepix = whitepix;
   header->align = align;
}

/**************************************************************/
int writeheader(FILE *fp, struct header_t *header)
{
   int n;

   n = fwrite((char *)header,1,HEADERSIZE,fp);
   if (n != HEADERSIZE) {
      (void) fprintf(stderr,
                     "%s: header fwrite returned %d, expected %d\n",
                     program,n,HEADERSIZE);
      return(-2);
   }

   return(0);
}

/************************************************************/
int readheader(FILE *fp, struct header_t *header)
{
   int n;

   n = fread((char *)header,1,HEADERSIZE,fp);
   if (n != HEADERSIZE) {
      (void) fprintf(stderr,
                     "%s: header fread returned %d, expected %d\n",
                     program,n,HEADERSIZE);
      return(-2);
   }

   return(0);
}

/*************************************************************/
int writedata(FILE *fp, u_char *data, int len)
{
   int n;

   n = fwrite((char *)data,1,len,fp);
   if (n != len) {
      (void) fprintf(stderr,
                     "%s: data fwrite returned %d, expected %d\n",
                     program,n,len);
      return(-2);
   }

   return(0);
}

/************************************************************/
int readdata(FILE *fp, u_char *data, int bytes)
{
   int n;

   n = fread((char *)data,1,bytes,fp);
   if (n != bytes) {
      (void) fprintf(stderr,
                     "%s: data fread returned %d, expected %d\n",
                     program,n,bytes);
      return(-2);
   }

   return(0);
}

/********************************************************************/
int fdclose(int fd, char *s)
{
   if (close(fd) < 0) {
      int e;
      char buffer[MAXPATHLEN];
      extern int errno;

      e = errno;
      (void) sprintf(buffer, "%s: cannot close fd %d (%s)", program,fd,s);
      errno = e;
      perror(buffer);
      return(-2);
   }

   return(0);
}

