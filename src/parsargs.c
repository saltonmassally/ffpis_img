/***********************************************************************
      LIBRARY: IMAGE - Image Manipulation and Processing Routines

      FILE:    PARSARGS.C

      AUTHORS: Criag Watson
               Michael Garris
      DATE:    02/15/2001

      Contains routines responsible for parsing command line
      argument relevant to image pixmap attributes.

      ROUTINES:
#cat: parse_w_h_d_ppi - takes a string of comma-separated image attributes
#cat:              and parses in order a width, height, depth, and
#cat:              optional ppi value.
#cat: parse_h_v_sampfctrs - takes a formatted string and parses
#cat:              component plane downsampling factors.

***********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <parsargs.h>
#include <jpegl.h>
#include <ffpis/util/util.h>

/*****************************************************************/
void parse_w_h_d_ppi(char *argstr, char *arg0,
                   int *width, int *height, int *depth, int *ppi)
{
   char cbuff[11], *cptr, *aptr;

   aptr = argstr;

   /* parse width */
   cptr = cbuff;
   while((*aptr != '\0') && (*aptr != ','))
      *cptr++ = *aptr++;
   if(*aptr == '\0'){
      print_usage(arg0);
      fprintf(stderr, "       height not found\n");
      exit(-1);
   }
   *cptr = '\0';
   *width = atoi(cbuff);

   /* parse height */
   cptr = cbuff;
   aptr++;
   while((*aptr != '\0') && (*aptr != ','))
      *cptr++ = *aptr++;
   if(*aptr == '\0'){
      print_usage(arg0);
      fprintf(stderr, "       depth not found\n");
      exit(-1);
   }
   *cptr = '\0';
   *height = atoi(cbuff);

   /* parse depth */
   cptr = cbuff;
   aptr++;
   while((*aptr != '\0') && (*aptr != ','))
      *cptr++ = *aptr++;
   *cptr = '\0';
   *depth = atoi(cbuff);

   if(*aptr != '\0'){
      /* parse ppi */
      cptr = cbuff;
      aptr++;
      while(*aptr != '\0')
         *cptr++ = *aptr++;
      *cptr = '\0';
      *ppi = atoi(cbuff);
   }
   /* Otherwise, no ppi passed. */
   else
      *ppi = -1;
}

/*****************************************************************/
void parse_h_v_sampfctrs(char *argstr, char *arg0,
                         int *hor_sampfctr, int *vrt_sampfctr, int *n_cmpnts)
{
   char cbuff[11], *cptr, *aptr;
   int t;

   aptr = argstr;

   *n_cmpnts = 0;

   while(*aptr != '\0'){
      if(*n_cmpnts > MAX_CMPNTS){
         print_usage(arg0);
         fprintf(stderr, "       number of components exceeds %d\n",
                 MAX_CMPNTS);
         exit(-1);
      }
      /* parse horizontal sample factor */
      cptr = cbuff;
      while((*aptr != '\0') && (*aptr != ','))
         *cptr++ = *aptr++;
      if(*aptr == '\0') {
         print_usage(arg0);
         fprintf(stderr, "       V[%d] not found\n", *n_cmpnts);
         exit(-1);
      }
      *cptr = '\0';
      t = hor_sampfctr[*n_cmpnts] = atoi(cbuff);
      if((t < 1) || (t > MAX_CMPNTS)){
         print_usage(arg0);
         fprintf(stderr, "       H[%d] = %d must be [1..%d]\n",
                 *n_cmpnts, t, MAX_CMPNTS);
         exit(-1);
      }
      aptr++;

      /* parse vertical sample factor */
      cptr = cbuff;
      while((*aptr != '\0') && (*aptr != ':'))
         *cptr++ = *aptr++;
      *cptr = '\0';
      t = vrt_sampfctr[*n_cmpnts] = atoi(cbuff);
      if((t < 1) || (t > MAX_CMPNTS)){
         print_usage(arg0);
         fprintf(stderr, "       V[%d] = %d must be [1..%d]\n",
                 *n_cmpnts, t, MAX_CMPNTS);
         exit(-1);
      }
      if(*aptr != '\0')
         aptr++;

      (*n_cmpnts)++;
   }
}
