/***********************************************************************
      LIBRARY: IMAGE - Image Manipulation and Processing Routines

      FILE:    IMGTYPE.C

      AUTHOR:  Criag Watson
      DATE:    01/16/2001

      Contains routines responsible for automatically determining the
      format of a pixmap file based on its contents.

      ROUTINES:
#cat: image_type - takes an image data stream and determines if it
#cat:              is from a WSQ, JPEGL, JPEGB, IHead, or UNKNOWN file.
#cat: jpeg_type - takes an image data stream and determines if it
#cat:             is from a JPEGL or JPEGB file.

***********************************************************************/
#include <stdio.h>
#include <string.h>
#include "imgtype.h"
#include "dataio.h"
#include "computil.h"

/*******************************************************************/
/* Determine if image data is of type IHEAD, WSQ, JPEGL, or JPEGB. */
/*******************************************************************/
int image_type(int *img_type, unsigned char *idata, const int ilen)
{
   int ret;
   unsigned short marker;
   unsigned char *cbufptr, *ebufptr;
   char ihdr_size[SHORT_CHARS];

   cbufptr = idata;
   ebufptr = idata + ilen;

   ret = getc_ushort(&marker, &cbufptr, ebufptr);
   if(ret)
      return(ret);
   if(marker == SOI_WSQ){
      *img_type = WSQ_IMG;
      return(0);
   }

   cbufptr = idata;
   ret = getc_ushort(&marker, &cbufptr, ebufptr);
   if(ret)
      return(ret);
   if(marker == SOI){
      ret = jpeg_type(img_type, idata, ilen);
      if(ret)
         return(ret);
      return(0);
   }

   sprintf(ihdr_size, "%d", IHDR_SIZE);
   if(strncmp(idata, ihdr_size, strlen(ihdr_size)) == 0){
      *img_type = IHEAD_IMG;
      return(0);
   }

   /* Otherwise, image type is UNKNOWN ... */
   *img_type = UNKNOWN_IMG;
   return(0);
}

/****************************************************************************/
/* Determines JPEG image type by finding SOF marker in the compressed data. */
/* The SOF's are different for each JPEG compression type JPEGL and JPEGB.  */
/****************************************************************************/
int jpeg_type(int *img_type, unsigned char *idata, const int ilen)
{
   int ret;
   unsigned short marker;
   unsigned char *cbufptr, *ebufptr;

   cbufptr = idata;
   ebufptr = idata + ilen;

   /* Get SOI */
   ret = getc_marker_jpegl(&marker, SOI, &cbufptr, ebufptr);
   if(ret)
      return(ret);

   /* Get next marker. */
   ret = getc_marker_jpegl(&marker, ANY, &cbufptr, ebufptr);
   if(ret)
      return(ret);

   /* While not at Start of Scan (SOS) -     */
   /*    the start of encoded image data ... */
   while(marker != SOS){
      if(marker == SOF3){
         *img_type = JPEGL_IMG;
         return(0);
      }
      else if(marker == SOF0){
         *img_type = JPEGB_IMG;
         return(0);
      }
      /* Skip marker segment. */
      ret = getc_skip_marker_segment(marker, &cbufptr, ebufptr);
      if(ret)
         return(ret);
      /* Get next marker. */
      ret = getc_marker_jpegl(&marker, ANY, &cbufptr, ebufptr);
      if(ret)
         return(ret);
   }

   /* JPEG type not found ... */
   fprintf(stderr, "ERROR : jpeg_type : Could not determine JPEG type ");
   fprintf(stderr, "(ie. baseline or lossless)\n");
   *img_type = -1;
   return(-2);
}
