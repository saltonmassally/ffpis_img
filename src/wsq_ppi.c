/***********************************************************************
      LIBRARY: WSQ - Grayscale Image Compression

      FILE:    PPI.C
      AUTHORS: Craig Watson
               Michael Garris
      DATE:    01/17/2001

      Contains routines responsible for determining the scan
      resolution of a WSQ compressed image by attempting to
      locate and parse a NISTCOM comment in the datastream.

      ROUTINES:
#cat: read_ppi_wsq - Given a WSQ compressed data stream, attempts to
#cat:                read a NISTCOM comment from an open file and
#cat:                if possible return the pixel scan resulution
#cat:                (PPI value) stored therein.
#cat: getc_ppi_wsq - Given a WSQ compressed data stream, attempts to
#cat:                read a NISTCOM comment from a memory buffer and
#cat:                if possible return the pixel scan resulution
#cat:                (PPI value) stored therein.

***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <wsq.h>

/************************************************************************/
int read_ppi_wsq(int *oppi, FILE *infp)
{
   int ret;
   long savepos;
   int ppi;
   char *value;
   NISTCOM *nistcom;

   /* Save current position in filestream ... */
   if((savepos = ftell(infp)) < 0){
      fprintf(stderr, "ERROR : read_ppi_wsq : ");
      fprintf(stderr, "ftell : couldn't determine current position\n");
      return(-2);
   }
   /* Set file pointer to beginning of filestream ... */
   if(fseek(infp, 0L, SEEK_SET) < 0){
      fprintf(stderr, "ERROR : read_ppi_wsq : ");
      fprintf(stderr, "fseek : couldn't set pointer to start of file\n");
      return(-3);
   }

   /* Get ppi from NISTCOM, if one exists ... */
   ret = read_nistcom_wsq(&nistcom, infp);
   if(ret){
      /* Reset file pointer to original position in filestream ... */
      if(fseek(infp, savepos, SEEK_SET) < 0){
         fprintf(stderr, "ERROR : read_ppi_wsq : ");
         fprintf(stderr, "fseek : couldn't reset file pointer\n");
         return(-4);
      }
      return(ret);
   }
   if(nistcom != (NISTCOM *)NULL){
      ret = extractfet_ret(&value, NCM_PPI, nistcom);
      if(ret){
         freefet(nistcom);
         /* Reset file pointer to original position in filestream ... */
         if(fseek(infp, savepos, SEEK_SET) < 0){
            fprintf(stderr, "ERROR : read_ppi_wsq : ");
            fprintf(stderr, "fseek : couldn't reset file pointer\n");
            return(-5);
         }
         return(ret);
      }
      if(value != (char *)NULL){
         ppi = atoi(value);
         free(value);
      }
      /* Otherwise, PPI not in NISTCOM, so ppi = -1. */
      else
         ppi = -1;
      freefet(nistcom);
   }
   /* Otherwise, NISTCOM does NOT exist, so ppi = -1. */
   else
      ppi = -1;

   /* Reset file pointer to original position in filestream ... */
   if(fseek(infp, savepos, SEEK_SET) < 0){
      fprintf(stderr, "ERROR : read_ppi_wsq : ");
      fprintf(stderr, "fseek : couldn't reset file pointer\n");
      return(-6);
   }

   *oppi = ppi;

   return(0);
}

/************************************************************************/
int getc_ppi_wsq(int *oppi, unsigned char *idata, const int ilen)
{
   int ret;
   int ppi;
   char *value;
   NISTCOM *nistcom;

   /* Get ppi from NISTCOM, if one exists ... */
   ret = getc_nistcom_wsq(&nistcom, idata, ilen);
   if(ret)
      return(ret);
   if(nistcom != (NISTCOM *)NULL){
      ret = extractfet_ret(&value, NCM_PPI, nistcom);
      if(ret){
         freefet(nistcom);
         return(ret);
      }
      if(value != (char *)NULL){
         ppi = atoi(value);
         free(value);
      }
      /* Otherwise, PPI not in NISTCOM, so ppi = -1. */
      else
         ppi = -1;
      freefet(nistcom);
   }
   /* Otherwise, NISTCOM does NOT exist, so ppi = -1. */
   else
      ppi = -1;

   *oppi = ppi;

   return(0);
}
