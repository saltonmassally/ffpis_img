/***********************************************************************
      LIBRARY: JPEGB - Baseline (Lossy) JPEG Utilities

      FILE:    MARKER.C
      AUTHORS: Michael Garris
               Craig Watson
      DATE:    01/16/2001

      Contains routines responsible for processing Baseline (lossy)
      JPEG markers.

      ROUTINES:
#cat: read_marker_jpegb - Reads a specified JPEG marker from
#cat:                     an open file.
#cat: getc_marker_jpegb - Reads a specified JPEG marker from a
#cat:                     memory buffer.
#cat: put_nistcom_jpegb - Inserts a NISTCOM comment into a JPEGB datastream.
#cat: read_nistcom_jpegb - Reads the first NISTCOM comment from an
#cat:                     open JPEGB file.
#cat: getc_nistcom_jpegb - Reads the first NISTCOM comment from a
#cat:                     memory buffer containing a JPEGB datastream.

***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <computil.h>
#include <jpegl.h>
#include <jpegb.h>
#include <swap.h>
#include <dataio.h>

/*****************************************/
/* Get markers from compressed open file */
/*****************************************/
int read_marker_jpegb(unsigned short *omarker, const int type, FILE *infp)
{
   int ret;
   unsigned short marker;

   ret = read_ushort(&marker, infp);
   if(ret)
      return(ret);

   switch(type){
   case SOI:
      if(marker != SOI) {
         fprintf(stderr,
         "ERROR : read_marker_jpegb : No SOI marker. {%d}\n", marker);
         return(-2);
      }
      break;
   case ANY:
      if((marker & 0xff00) != 0xff00){
	fprintf(stderr,"ERROR : read_marker_jpegb : no marker found {%04X}\n",
                marker);
         return(-3);
      }
      break;
   default:
      fprintf(stderr,
      "ERROR : read_marker_jpegb : invalid marker case -> {%4X}\n", type);
      return(-4);
   }

   *omarker =  marker;
   return(0);
}

/*********************************************/
/* Get markers from compressed memory buffer */
/*********************************************/
int getc_marker_jpegb(unsigned short *omarker, const int type,
                      unsigned char **cbufptr, unsigned char *ebufptr)
{
   int ret;
   unsigned short marker;

   ret = getc_ushort(&marker, cbufptr, ebufptr);
   if(ret)
      return(ret);

   switch(type){
   case SOI:
      if(marker != SOI) {
         fprintf(stderr,
         "ERROR : getc_marker_jpegb : No SOI marker. {%d}\n", marker);
         return(-2);
      }
      break;
   case ANY:
      if((marker & 0xff00) != 0xff00){
	fprintf(stderr,"ERROR : getc_marker_jpegb : no marker found {%04X}\n",
                marker);
         return(-3);
      }
      break;
   default:
      fprintf(stderr,
      "ERROR : getc_marker_jpegb : invalid marker case -> {%4X}\n", type);
      return(-4);
   }

   *omarker =  marker;
   return(0);
}

/*******************************************/
int put_nistcom_jpegb(j_compress_ptr cinfo, char *comment_text,
                       const int w, const int h, const int d, const int ppi,
                       const int lossyflag, const int quality)
{
   int ret, gencomflag;
   NISTCOM *nistcom;
   char *comstr, *colorspace;
   int n_cmpnts;

   /* Add Comment(s) here. */
   nistcom = (NISTCOM *)NULL;
   gencomflag = 0;
   if(comment_text != (char *)NULL){
      /* if NISTCOM ... */
      if(strncmp(comment_text, NCM_HEADER, strlen(NCM_HEADER)) == 0){
         ret = string2fet(&nistcom, comment_text);
         if(ret){
            return(ret);
         }
      }
      /* If general comment ... */
      else{
         gencomflag = 1;
      }
   }
   /* Otherwise, no comment passed ... */

   n_cmpnts = cinfo->input_components;
   if(n_cmpnts == 1)
      colorspace = "GRAY";
   else if(n_cmpnts == 3)
      colorspace = "YCbCr";
   else{
      fprintf(stderr, "ERROR : put_nistcom_jpegb : \n");
      fprintf(stderr, "number of components = %d != {1,3}\n", n_cmpnts);
      if(nistcom != (NISTCOM *)NULL)
         freefet(nistcom);
      return(-2);
   }

   /* Combine image attributes to NISTCOM. */
   ret = combine_jpegb_nistcom(&nistcom, w, h, d, ppi, lossyflag,
		   colorspace, n_cmpnts, 1 /* intrlv */, quality);
   if(ret){
      if(nistcom != (NISTCOM *)NULL)
         freefet(nistcom);
      return(ret);
   }

   /* Put NISTCOM ... */
   /* NISTCOM to string. */
   ret = fet2string(&comstr, nistcom);
   if(ret){
      freefet(nistcom);
      return(ret);
   }
   /* Put NISTCOM comment string. */
   jpeg_write_marker(cinfo, JPEG_COM, comstr, strlen(comstr));
   freefet(nistcom);
   free(comstr);

   /* If general comment exists ... */
   if(gencomflag){
      /* Put general comment to its own segment. */
      jpeg_write_marker(cinfo, JPEG_COM, comment_text, strlen(comment_text));
   }

   return(0);
}

/***************************************************************/
/* Get and return first NISTCOM from encoded open file.        */
/***************************************************************/
int read_nistcom_jpegb(NISTCOM **onistcom, FILE *infp)
{
   int ret;
   long savepos;
   unsigned short marker;
   NISTCOM *nistcom;
   char *value, *comment_text;
   int id_len;

   /* Get SOI */
   ret = read_marker_jpegb(&marker, SOI, infp);
   if(ret)
      return(ret);

   /* Get next marker. */
   ret = read_marker_jpegb(&marker, ANY, infp);
   if(ret)
      return(ret);

   /* Allocate temporary buffer the size of the NIST_COM Header ID. */
   id_len = strlen(NCM_HEADER);
   value = (char *)calloc(id_len+1, sizeof(char));
   if(value == (char *)NULL){
      fprintf(stderr, "ERROR : read_nistcom_jpegb : calloc : value\n");
      return(-2);
   }

   /* While not at Start of Scan (SOS) -     */
   /*    the start of encoded image data ... */
   while(marker != SOS){
      if(marker == COM){
         if((savepos = ftell(infp)) < 0){
            fprintf(stderr, "ERROR : read_nistcom_jpegb : ");
            fprintf(stderr, "ftell : unable to determine current position\n");
            free(value);
            return(-3);
         }
         /* Skip Length (short) Bytes */
         if(fseek(infp, 2L, SEEK_CUR) < 0){
            fprintf(stderr, "ERROR : read_nistcom_jpegb : ");
            fprintf(stderr, "fseek : unable to skip length bytes\n");
            free(value);
            return(-4);
         }
         /* Should be a safe assumption here that we can read */
         /* id_len bytes without reaching EOF, so if we don't */
         /* read all id_len bytes, then flag error.           */
         ret = fread(value, sizeof(char), id_len, infp);
         if(ret != id_len){
            fprintf(stderr, "ERROR : read_nistcom_jpegb : ");
            fprintf(stderr, "fread : only %d of %d bytes read\n",
                    ret, id_len);
            free(value);
            return(-5);
         }
         /* Reset file pointer to original position. */
         if(fseek(infp, savepos, SEEK_SET) < 0){
            fprintf(stderr, "ERROR : read_nistcom_jpegb : ");
            fprintf(stderr, "fseek : unable to reset file position\n");
            free(value);
            return(-6);
         }
         if(strncmp(value, NCM_HEADER, id_len) == 0){
            ret = read_comment((unsigned char **)&comment_text, infp);
            if(ret){
               free(value);
               return(ret);
            }
	    ret = string2fet(&nistcom, comment_text);
            if(ret){
               free(value);
               return(ret);
            }
            free(value);
            *onistcom = nistcom;
            return(0);
         }
      }
      /* Skip marker segment. */
      ret = read_skip_marker_segment(marker, infp);
      if(ret){
         free(value);
         return(ret);
      }
      /* Get next marker. */
      ret = read_marker_jpegb(&marker, ANY, infp);
      if(ret){
         free(value);
         return(ret);
      }
   }

   free(value);

   /* NISTCOM not found ... */
   *onistcom = (NISTCOM *)NULL;
   return(0);
}

/*****************************************************************/
/* Get and return first NISTCOM from encoded data stream.        */
/*****************************************************************/
int getc_nistcom_jpegb(NISTCOM **onistcom, unsigned char *idata,
                        const int ilen)
{
   int ret;
   unsigned short marker;
   unsigned char *cbufptr, *ebufptr;
   NISTCOM *nistcom;
   unsigned char *comment_text;

   cbufptr = idata;
   ebufptr = idata + ilen;

   /* Get SOI */
   ret = getc_marker_jpegb(&marker, SOI, &cbufptr, ebufptr);
   if(ret)
      return(ret);

   /* Get next marker. */
   ret = getc_marker_jpegb(&marker, ANY, &cbufptr, ebufptr);
   if(ret)
      return(ret);

   /* While not at Start of Scan (SOS) -     */
   /*    the start of encoded image data ... */
   while(marker != SOS){
      if(marker == COM){
         if(strncmp(cbufptr+2 /* skip Length */,
                    NCM_HEADER, strlen(NCM_HEADER)) == 0){
            ret = getc_comment(&comment_text, &cbufptr, ebufptr);
            if(ret)
               return(ret);
	    ret = string2fet(&nistcom, comment_text);
            if(ret)
               return(ret);
            *onistcom = nistcom;
            return(0);
         }
      }
      /* Skip marker segment. */
      ret = getc_skip_marker_segment(marker, &cbufptr, ebufptr);
      if(ret)
         return(ret);
      /* Get next marker. */
      ret = getc_marker_jpegb(&marker, ANY, &cbufptr, ebufptr);
      if(ret)
         return(ret);
   }

   /* NISTCOM not found ... */
   *onistcom = (NISTCOM *)NULL;
   return(0);
}
