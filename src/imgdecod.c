/***********************************************************************
      LIBRARY: IMAGE - Image Manipulation and Processing Routines

      FILE:    IMGDECOD.C
      AUTHORS: Michael Garris
               
      DATE:    03/08/2001

      Contains routines responsible for taking a formatted datastream
      of potentially compressed image pixels, identifying the format
      type of the datastream if possible, and then decoding the
      datastream returning a reconstructed pixmap.

      ROUTINES:
#cat: read_and_decode_dpyimage - identifies and reconstructs a
#cat:          potentially compressed datastream of image pixels
#cat:          for use by the display application "dpyimage".
#cat: read_and_decode_image - identifies and reconstructs a
#cat:          potentially compressed datastream of image pixels.
#cat: ihead_decode_mem - decodes (if necessary) a datastream of
#cat:          IHead formatted pixels from a memory buffer.

***********************************************************************/
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <img_io.h>
#include <imgtype.h>
#include <wsq.h>
#ifdef jpegb_SUPPORTED
#include <jpegb.h>
#endif
#include <jpegl.h>
#include <ihead.h>
#include "getnset.h"
#include "imgutil.h"
#include "grp4deco.h"
#include "invbyte.h"
#include "intrlv.h"
#include <imgdecod.h>

/*******************************************************************/
int read_and_decode_dpyimage(char *ifile, int *oimg_type,
                    unsigned char **odata, int *olen,
                    int *ow, int *oh, int *od, int *oppi)
{
   int ret, i, found;
   unsigned char *idata, *ndata;
   int img_type, ilen, nlen;
   int w, h, d, ppi;
   int intrlvflag;
   int hor_sampfctr[MAX_CMPNTS], vrt_sampfctr[MAX_CMPNTS], n_cmpnts;

   *odata = (unsigned char *)NULL;
   *olen = 0;

   /* Read in and decode image file. */
   ret = read_and_decode_image(ifile, &img_type, &idata, &ilen, &w, &h, &d,
		   &ppi, &intrlvflag, hor_sampfctr, vrt_sampfctr, &n_cmpnts);
   if(ret){
      return(ret);
   }

   /* If image type is UNKNOWN ... simply return what was read. */
   if(img_type == UNKNOWN_IMG){
      *oimg_type = img_type;
      *odata = idata;
      *olen = ilen;
      *ow = w;
      *oh = h;
      *od = d;
      *oppi = ppi;

      return(0);
   }

   /* Dpyimage can only handle these pixel depths ... */
   if((d != 1) && (d != 8) && (d != 24)){
      fprintf(stderr, "WARNING : read_and_decode_dpyimage : ");
      fprintf(stderr, "file %s IGNORED : pixdepth = %d != {1,8,24}\n",
              ifile, d);
      return(IMG_IGNORE);
   }

   /* Dpyimage cannot handle downsampled component planes. */
   if((img_type == JPEGL_IMG) && (n_cmpnts > 1)){
      found = 0;
      for(i = 0; i < n_cmpnts; i++){
         if((hor_sampfctr[i] != 1) ||
            (vrt_sampfctr[i] != 1)){
             found = 1;
             break;
         }
      }
      if(found){
         fprintf(stderr, "WARNING : read_and_decode_dpyimage : ");
         fprintf(stderr, "file %s IGNORED : ", ifile);
         fprintf(stderr, "contains HV sample factor(s) != 1\n");
         return(IMG_IGNORE);
      }
   }

   /* Dpyimage cannot handle non-interleaved pixel data, */
   /* so interleave the image data if necessary. */
   if((d == 24) && (!intrlvflag)){
      ret = not2intrlv_mem(&ndata, &nlen, idata, w, h, d, hor_sampfctr,
		      vrt_sampfctr, n_cmpnts);
      if(ret){
         free(idata);
         return(ret);
      }
      free(idata);
      idata = ndata;
      ilen = nlen;
   }

   *oimg_type = img_type;
   *odata = idata;
   *olen = ilen;
   *ow = w;
   *oh = h;
   *od = d;
   *oppi = ppi;

   return(0);
}

/*******************************************************************/
int read_and_decode_image(char *ifile, int *oimg_type,
                    unsigned char **odata, int *olen,
                    int *ow, int *oh, int *od, int *oppi, int *ointrlvflag,
                    int *hor_sampfctr, int *vrt_sampfctr, int *on_cmpnts)
{
   int ret, i;
   unsigned char *idata, *ndata;
   int img_type, ilen, nlen;
   int w, h, d, ppi;
   int lossyflag, intrlvflag=0, n_cmpnts;
   IMG_DAT *img_dat;

   ret = read_raw_from_filesize(ifile, &idata, &ilen);
   if(ret)
      return(ret);

   ret = image_type(&img_type, idata, ilen);
   if(ret){
      free(idata);
      return(ret);
   }

   switch(img_type){
      case UNKNOWN_IMG:
           /* Return raw image data as read from file. */
           *oimg_type = img_type;
           *odata = idata;
           *olen = ilen;
           *ow = -1;
           *oh = -1;
           *od = -1;
           *oppi = -1;
           *ointrlvflag = -1;
           *on_cmpnts = -1;
           return(0);
      case WSQ_IMG:
	   ret = wsq_decode_mem(&ndata, &w, &h, &d, &ppi,
			   &lossyflag, idata, ilen);
           if(ret){
              free(idata);
              return(ret);
           }
           nlen = w * h;
           /* Pix depth always 8 for WSQ ... */
           n_cmpnts = 1;
           hor_sampfctr[0] = 1;
           vrt_sampfctr[0] = 1;
           break;
      case JPEGL_IMG:
	   ret = jpegl_decode_mem(&img_dat, &lossyflag, idata, ilen);
           if(ret){
              free(idata);
              return(ret);
           }
	   ret = get_IMG_DAT_image(&ndata, &nlen, &w, &h, &d, &ppi, img_dat);
           if(ret){
              free(idata);
              free_IMG_DAT(img_dat, FREE_IMAGE);
              return(ret);
           }
           /* JPEGL always returns non-interleaved data. */
           intrlvflag = 0;
           n_cmpnts = img_dat->n_cmpnts;
           if(d == 24){
              for(i = 0; i < n_cmpnts; i++){
                 hor_sampfctr[i] = img_dat->hor_sampfctr[i];
                 vrt_sampfctr[i] = img_dat->vrt_sampfctr[i];
              }
           }
           free_IMG_DAT(img_dat, FREE_IMAGE);
           break;
      case JPEGB_IMG:
#if jpegb_SUPPORTED
	   ret = jpegb_decode_mem(&ndata, &w, &h, &d, &ppi,
			   &lossyflag, idata, ilen);
           if(ret){
              free(idata);
              return(ret);
           }
           if(d == 8){
              n_cmpnts = 1;
              intrlvflag = 0;
           }
           else if(d == 24){
              n_cmpnts = 3;
              intrlvflag = 1;
           }
           else{
              fprintf(stderr, "ERROR : read_and_decode_image : ");
              fprintf(stderr, "JPEGB decoder returned d=%d ", d);
              fprintf(stderr, "not equal to 8 or 24\n");
              free(idata);
              return(-2);
           }
           nlen = w * h * (d>>3);
           for(i = 0; i < n_cmpnts; i++){
              hor_sampfctr[i] = 1;
              vrt_sampfctr[i] = 1;
           }
           break;
#else
	   return -2;
#endif
      case IHEAD_IMG:
	   ret = ihead_decode_mem(&ndata, &w, &h, &d, &ppi,
			   &lossyflag, idata, ilen);
           if(ret){
              free(idata);
              return(ret);
           }

           nlen = SizeFromDepth(w,h,d);
           if((d == 1) || (d == 8)){
              n_cmpnts = 1;
              intrlvflag = 0;
           }
           else if(d == 24){
              n_cmpnts = 3;
              intrlvflag = 1;
           }
           else{
              fprintf(stderr, "ERROR : read_and_decode_image : ");
              fprintf(stderr, "IHead decoder returned d=%d ", d);
              fprintf(stderr, "not equal to {1,8,24}\n");
              free(idata);
              return(-2);
           }
           for(i = 0; i < n_cmpnts; i++){
              hor_sampfctr[i] = 1;
              vrt_sampfctr[i] = 1;
           }
           break;
      default:
           fprintf(stderr, "ERROR : read_and_decode_image : ");
           fprintf(stderr, "illegal image type = %d\n", img_type);
           return(-3);
   }

   free(idata);

   *oimg_type = img_type;
   *odata = ndata;
   *olen = nlen;
   *ow = w;
   *oh = h;
   *od = d;
   *oppi = ppi;
   *ointrlvflag = intrlvflag;
   *on_cmpnts = n_cmpnts;

   return(0);
}

/*******************************************************************/
int ihead_decode_mem(unsigned char **oodata, int *ow, int *oh, int *od,
                     int *oppi, int *lossyflag,
                     unsigned char *idata, const int ilen)
{
   IHEAD *ihead;
   unsigned char *odata, *iptr;
   int olen, obytes, w, h, d, ppi;
   int compcode, complen=0;

   (void)ilen; /* FIXME unused */
   /* Skip first fized length size field. */
   iptr = idata + SHORT_CHARS;
   ihead = (IHEAD *)iptr;
   iptr += sizeof(IHEAD);

   w = get_width(ihead);
   h = get_height(ihead);
   d = get_depth(ihead);
   ppi = get_density(ihead);
   compcode = get_compression(ihead);

   olen = SizeFromDepth(w,h,d);
   odata = (unsigned char *)malloc(olen);
   if(odata == (unsigned char *)NULL){
      fprintf(stderr, "ERROR : ihead_decode_mem : malloc : odata\n");
      return(-2);
   }

   if(compcode != UNCOMP){
      complen = get_complen(ihead);
   }

   switch (compcode) {
      case RL:
         rldecomp(iptr, complen, odata, &obytes, olen);
         set_compression(ihead, UNCOMP);
         set_complen(ihead, 0);
         break;
      case CCITT_G4:
         if(ihead->sigbit == LSBF) {
           inv_bytes(iptr, complen);
           ihead->sigbit = MSBF;
           ihead->byte_order = HILOW;
         }
         grp4decomp(iptr, complen, w, h, odata, &obytes);
         set_compression(ihead, UNCOMP);
         set_complen(ihead, 0);
         break;
      case UNCOMP:
         memcpy(odata, iptr, olen);
      break;
      default:
         fprintf(stderr, "ERROR : ihead_decode_mem : ");
         fprintf(stderr, "invalid compression code = %d\n", compcode);
         return(-3);
      break;
   }

   *oodata = odata;
   *ow = w;
   *oh = h;
   *od = d;
   *oppi = ppi;
   *lossyflag = 0;

   return(0);
}
