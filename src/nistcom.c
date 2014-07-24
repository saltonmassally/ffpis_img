/***********************************************************************
      LIBRARY: FET - Feature File/List Utilities

      FILE:    NISTCOM.C
      AUTHORS: Michael Garris
               Craig Watson
      DATE:    02/27/2001

      Contains routines responsible for manipulating a specialize
      attribute-value list called a NISTCOM used to hold image
      attributes relevant to image compression, decompression, and
      classification.

      ROUTINES:
#cat: combine_nistcom - takes an initialized FET NISTCOM structure
#cat:             or allocates one if necessary, and updates the
#cat:             structure with general image attributes.
#cat: combine_jpegl_nistcom - takes an initialized FET NISTCOM structure
#cat:             or allocates one if necessary, and updates the
#cat:             structure with JPEGL-specific image attributes.
#cat: combine_wsq_nistcom - takes an initialized FET NISTCOM structure
#cat:             or allocates one if necessary, and updates the
#cat:             structure with WSQ-specific image attributes.
#cat: combine_jpegb_nistcom - takes an initialized FET NISTCOM structure
#cat:             or allocates one if necessary, and updates the
#cat:             structure with JPEGB-specific image attributes.
#cat: del_jpegl_nistcom - takes an initialized FET NISTCOM structure
#cat:             and removes JPEGL compression attributes.
#cat: del_wsq_nistcom - takes an initialized FET NISTCOM structure
#cat:             and removes WSQ compression attributes.
#cat: del_jpegb_nistcom - takes an initialized FET NISTCOM structure
#cat:             and removes JPEGB compression attributes.
#cat: sd_ihead_to_nistcom - takes an IHead header from a specified NIST
#cat:             Special Database file, and parses specific attribute data
#cat:             from the header into an FET NISTCOM structure.
#cat: sd4_ihead_to_nistcom - takes an IHead header from a NIST Special
#cat:             Database 4 file, and parses specific attribute data
#cat:             from the header into an FET NISTCOM structure.
#cat: sd9_10_14_ihead_to_nistcom - takes an IHead header from a NIST Special
#cat:             Database 9,10,14 file, and parses specific attribute data
#cat:             from the header into an FET NISTCOM structure.
#cat: sd18_ihead_to_nistcom - takes an IHead header from a NIST Special
#cat:             Database 18 file, and parses specific attribute data
#cat:             from the header into an FET NISTCOM structure.
#cat: get_sd_class - gets the class from a special database id field
#cat:
#cat: get_class_from_ncic_class_string - gets the class from a special
#cat:             database ncic class string

***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ihead.h>
#include <nistcom.h>

/*****************************************************************/
int combine_nistcom(NISTCOM **onistcom, const int w, const int h,
                     const int d, const int ppi, const int lossyflag)
{
   int ret, allocflag, n;
   char *lossyval;
   char cbuff[11];
   NISTCOM *nistcom;


   /* ALLOCATION ? */
   if((*onistcom) == (NISTCOM *)NULL){
      ret = allocfet_ret(&nistcom, 6);
      if(ret)
         return(ret);
      allocflag = 1;
      /* HEADER */
      ret = updatefet_ret(NCM_HEADER, "6", nistcom);
      if(ret){
         if(allocflag){
            freefet(nistcom);
            *onistcom = (NISTCOM *)NULL;
         }
         return(ret);
      }
   }
   else{
      nistcom = *onistcom;
      allocflag = 0;
      n = 6;
   }

   /* WIDTH */
   sprintf(cbuff, "%d", w);
   ret = updatefet_ret(NCM_PIX_WIDTH, cbuff, nistcom);
   if(ret){
      if(allocflag){
         freefet(nistcom);
         *onistcom = (NISTCOM *)NULL;
      }
      return(ret);
   }

   /* HEIGHT */
   sprintf(cbuff, "%d", h);
   ret = updatefet_ret(NCM_PIX_HEIGHT, cbuff, nistcom);
   if(ret){
      if(allocflag){
         freefet(nistcom);
         *onistcom = (NISTCOM *)NULL;
      }
      return(ret);
   }

   /* DEPTH */
   sprintf(cbuff, "%d", d);
   ret = updatefet_ret(NCM_PIX_DEPTH, cbuff, nistcom);
   if(ret){
      if(allocflag){
         freefet(nistcom);
         *onistcom = (NISTCOM *)NULL;
      }
      return(ret);
   }

   /* PPI */
   sprintf(cbuff, "%d", ppi);
   ret = updatefet_ret(NCM_PPI, cbuff, nistcom);
   if(ret){
      if(allocflag){
         freefet(nistcom);
         *onistcom = (NISTCOM *)NULL;
      }
      return(ret);
   }

   /* LOSSY */
   /* If exists, lookup current LOSSY value. */
   ret = lookupfet(&lossyval, NCM_LOSSY, nistcom);
   /* If error ... */
   if(ret < 0){
      if(allocflag){
         freefet(nistcom);
         *onistcom = (NISTCOM *)NULL;
      }
      return(ret);
   }
   /* If LOSSY value found AND is set AND requesting to unset ... */
   if(ret &&
     (strcmp(lossyval,"0") != 0) &&
     (lossyflag == 0)){
      fprintf(stderr, "WARNING : combine_nistcom : ");
      fprintf(stderr, "request to unset lossy flag ignored\n");
   }
   else{
      sprintf(cbuff, "%d", lossyflag);
      ret = updatefet_ret(NCM_LOSSY, cbuff, nistcom);
      if(ret){
         if(allocflag){
            freefet(nistcom);
            *onistcom = (NISTCOM *)NULL;
         }
         return(ret);
      }
   }

   /* UPDATE HEADER */
   sprintf(cbuff, "%d", nistcom->num);
   ret = updatefet_ret(NCM_HEADER, cbuff, nistcom);
   if(ret){
      if(allocflag){
         freefet(nistcom);
         *onistcom = (NISTCOM *)NULL;
      }
      return(ret);
   }

   *onistcom = nistcom;

   return(0);
}

/*****************************************************************/
int combine_jpegl_nistcom(NISTCOM **onistcom, const int w, const int h,
                  const int d, const int ppi, const int lossyflag,
                  const int n_cmpnts, int *hor_sampfctr, int *vrt_sampfctr,
                  const int intrlvflag, const int predict)
{
   int ret, i, allocflag;
   NISTCOM *nistcom;
   char cbuff[MAXFETLENGTH], *cptr;

   if(*onistcom == (NISTCOM *)NULL)
      allocflag = 1;
   else
      allocflag = 0;

   /* Combine image attributes to NISTCOM. */
   ret = combine_nistcom(onistcom, w, h, d, ppi, lossyflag);
   if(ret)
      return(ret);

   nistcom = *onistcom;

   /* COLORSPACE - only sure of GRAY */
   if(n_cmpnts == 1){
      ret = updatefet_ret(NCM_COLORSPACE, "GRAY", nistcom);
      if(ret){
         if(allocflag)
            freefet(nistcom);
         return(ret);
      }
   }

   if(n_cmpnts > 1){
      /* NUM_COMPONENTS */
      sprintf(cbuff, "%d", n_cmpnts);
      ret = updatefet_ret(NCM_N_CMPNTS, cbuff, nistcom);
      if(ret){
         if(allocflag)
            freefet(nistcom);
         return(ret);
      }

      /* HV FACTORS */
      cptr = cbuff;
      sprintf(cptr, "%d,%d", hor_sampfctr[0], vrt_sampfctr[0]);
      cptr = cbuff + strlen(cbuff);
      for(i = 1; i < n_cmpnts; i++){
         sprintf(cptr, ":%d,%d", hor_sampfctr[i], vrt_sampfctr[i]);
         cptr = cbuff + strlen(cbuff);
      }

      ret = updatefet_ret(NCM_HV_FCTRS, cbuff, nistcom);
      if(ret){
         if(allocflag)
            freefet(nistcom);
         return(ret);
      }

      /* INTERLEAVE */
      sprintf(cbuff, "%d", intrlvflag);
      ret = updatefet_ret(NCM_INTRLV, cbuff, nistcom);
      if(ret){
         if(allocflag)
            freefet(nistcom);
         return(ret);
      }
   }

   /* COMPRESSION */
   ret = updatefet_ret(NCM_COMPRESSION, "JPEGL", nistcom);
   if(ret){
      if(allocflag)
         freefet(nistcom);
      return(ret);
   }

   /* PREDICT */
   sprintf(cbuff, "%d", predict);
   ret = updatefet_ret(NCM_JPEGL_PREDICT, cbuff, nistcom);
   if(ret){
      if(allocflag)
         freefet(nistcom);
      return(ret);
   }

   /* UPDATE HEADER */
   sprintf(cbuff, "%d", nistcom->num);
   ret = updatefet_ret(NCM_HEADER, cbuff, nistcom);
   if(ret){
      if(allocflag)
         freefet(nistcom);
      return(ret);
   }

   return(0);
}

/*****************************************************************/
int combine_wsq_nistcom(NISTCOM **onistcom, const int w, const int h,
                  const int d, const int ppi, const int lossyflag,
                  const float r_bitrate)
{
   int ret, allocflag;
   NISTCOM *nistcom;
   char cbuff[MAXFETLENGTH];

   if(*onistcom == (NISTCOM *)NULL)
      allocflag = 1;
   else
      allocflag = 0;

   /* Combine image attributes to NISTCOM. */
   ret = combine_nistcom(onistcom, w, h, d, ppi, lossyflag);
   if(ret)
      return(ret);

   nistcom = *onistcom;

   /* COLORSPACE */
   ret = updatefet_ret(NCM_COLORSPACE, "GRAY", nistcom);
   if(ret){
      if(allocflag)
         freefet(nistcom);
      return(ret);
   }

   /* COMPRESSION */
   ret = updatefet_ret(NCM_COMPRESSION, "WSQ", nistcom);
   if(ret){
      if(allocflag)
         freefet(nistcom);
      return(ret);
   }

   /* BITRATE */
   sprintf(cbuff, "%f", r_bitrate);
   ret = updatefet_ret(NCM_WSQ_RATE, cbuff, nistcom);
   if(ret){
      if(allocflag)
         freefet(nistcom);
      return(ret);
   }

   /* UPDATE HEADER */
   sprintf(cbuff, "%d", nistcom->num);
   ret = updatefet_ret(NCM_HEADER, cbuff, nistcom);
   if(ret){
      if(allocflag)
         freefet(nistcom);
      return(ret);
   }

   return(0);
}

/*****************************************************************/
int combine_jpegb_nistcom(NISTCOM **onistcom, const int w, const int h,
                  const int d, const int ppi, const int lossyflag,
                  char *colorspace, const int n_cmpnts, const int intrlvflag,
                  const int quality)
{
   int ret, allocflag;
   NISTCOM *nistcom;
   char cbuff[MAXFETLENGTH], *cptr;

   if(*onistcom == (NISTCOM *)NULL)
      allocflag = 1;
   else
      allocflag = 0;

   /* Combine image attributes to NISTCOM. */
   ret = combine_nistcom(onistcom, w, h, d, ppi, lossyflag);
   if(ret)
      return(ret);

   nistcom = *onistcom;

   /* COLORSPACE */
   ret = updatefet_ret(NCM_COLORSPACE, colorspace, nistcom);
   if(ret){
      if(allocflag)
         freefet(nistcom);
      return(ret);
   }

   if(n_cmpnts > 1){
      /* NUM_COMPONENTS */
      sprintf(cbuff, "%d", n_cmpnts);
      ret = updatefet_ret(NCM_N_CMPNTS, cbuff, nistcom);
      if(ret){
         if(allocflag)
            freefet(nistcom);
         return(ret);
      }

      /* HV_FACTORS */
      if(strcmp(colorspace, "RGB") == 0)
         cptr = "1,1:1,1:1,1";
      else if(strcmp(colorspace, "YCbCr") == 0)
         cptr = "2,2:1,1:1,1";
      else{
         fprintf(stderr, "ERROR : combine_jpegb_nistcom : ");
         fprintf(stderr, "unknown/unsupported colorspace = %s\n",
                         colorspace);
         if(allocflag)
            freefet(nistcom);
         return(-2);
      }
      ret = updatefet_ret(NCM_HV_FCTRS, cptr, nistcom);
      if(ret){
         if(allocflag)
            freefet(nistcom);
         return(ret);
      }

      /* INTERLEAVE */
      sprintf(cbuff, "%d", intrlvflag);
      ret = updatefet_ret(NCM_INTRLV, cbuff, nistcom);
      if(ret){
         if(allocflag)
            freefet(nistcom);
         return(ret);
      }
   }

   /* COMPRESSION */
   ret = updatefet_ret(NCM_COMPRESSION, "JPEGB", nistcom);
   if(ret){
      if(allocflag)
         freefet(nistcom);
      return(ret);
   }

   /* QUALITY */
   sprintf(cbuff, "%d", quality);
   ret = updatefet_ret(NCM_JPEGB_QUAL, cbuff, nistcom);
   if(ret){
      if(allocflag)
         freefet(nistcom);
      return(ret);
   }

   /* UPDATE HEADER */
   sprintf(cbuff, "%d", nistcom->num);
   ret = updatefet_ret(NCM_HEADER, cbuff, nistcom);
   if(ret){
      if(allocflag)
         freefet(nistcom);
      return(ret);
   }

   return(0);
}

/*****************************************************************/
int del_jpegl_nistcom(NISTCOM *nistcom)
{
   int ret;
   char cbuff[MAXFETLENGTH];

   /* COMPRESSION */
   ret = deletefet_ret(NCM_COMPRESSION, nistcom);
   if(ret)
      return(ret);

   /* PREDICT */
   ret = deletefet_ret(NCM_JPEGL_PREDICT, nistcom);
   if(ret)
      return(ret);

   /* UPDATE HEADER */
   sprintf(cbuff, "%d", nistcom->num);
   ret = updatefet_ret(NCM_HEADER, cbuff, nistcom);
   if(ret)
      return(ret);

   return(0);
}

/*****************************************************************/
int del_wsq_nistcom(NISTCOM *nistcom)
{
   int ret;
   char cbuff[MAXFETLENGTH];

   /* COMPRESSION */
   ret = deletefet_ret(NCM_COMPRESSION, nistcom);
   if(ret)
      return(ret);

   /* BITRATE */
   ret = deletefet_ret(NCM_WSQ_RATE, nistcom);
   if(ret)
      return(ret);

   /* UPDATE HEADER */
   sprintf(cbuff, "%d", nistcom->num);
   ret = updatefet_ret(NCM_HEADER, cbuff, nistcom);
   if(ret)
      return(ret);

   return(0);
}

/*****************************************************************/
int del_jpegb_nistcom(NISTCOM *nistcom)
{
   int ret;
   char cbuff[MAXFETLENGTH];

   /* COMPRESSION */
   ret = deletefet_ret(NCM_COMPRESSION, nistcom);
   if(ret)
      return(ret);

   /* QUALITY */
   ret = deletefet_ret(NCM_JPEGB_QUAL, nistcom);
   if(ret)
      return(ret);

   /* UPDATE HEADER */
   sprintf(cbuff, "%d", nistcom->num);
   ret = updatefet_ret(NCM_HEADER, cbuff, nistcom);
   if(ret)
      return(ret);

   return(0);
}

/************************************/
/* Puts database Ihead into NISTCOM */
/************************************/
int sd_ihead_to_nistcom(NISTCOM **nistcom, IHEAD *ihead, int sd_id)
{
   switch(sd_id) {
      case 4:
         return(sd4_ihead_to_nistcom(nistcom, ihead));
         break;
      case 9:
         return(sd9_10_14_ihead_to_nistcom(nistcom, ihead, sd_id));
         break;
      case 10:
         return(sd9_10_14_ihead_to_nistcom(nistcom, ihead, sd_id));
         break;
      case 14:
         return(sd9_10_14_ihead_to_nistcom(nistcom, ihead, sd_id));
         break;
      case 18:
         return(sd18_ihead_to_nistcom(nistcom, ihead));
         break;
      default:
         fprintf(stderr,
                 "ERROR : sd_ihead_to_nistcom : invalid database id = %d\n",
                 sd_id);
         fprintf(stderr, "        expecting SD 4,9,10,14, or 18\n");
         *nistcom = '\0';
         return(-2);
   }
}

/*******************************/
/* Puts SD4 Ihead into NISTCOM */
/*******************************/
int sd4_ihead_to_nistcom(NISTCOM **onistcom, IHEAD *ihead)
{
   char *hst, *fname, *class_str, class, *sex, *pname;
   int ret, hst_sz;
   NISTCOM *nistcom;
   char cbuff[11], id_str[BUFSIZE];

   /** Get information needed from NIST IHEAD Structure **/
   strcpy(id_str, ihead->id);
   id_str[12] = '\0';
   fname = id_str;

   if(id_str[14] == ' ')
      id_str[14] = '\0';
   else
      id_str[15] = '\0';
   class_str = &(id_str[13]);

   sex = &(id_str[16]);

   pname = ihead->parent;

   /* size of HISTORY field "<fname> <class_str> <pname>\0" */
   hst_sz = 2 + strlen(fname) + strlen(class_str) + strlen(pname);

   if((hst = (char *)malloc(hst_sz)) == (char *)NULL){
      fprintf(stderr, "ERROR : sd4_ihead_to_nistcom : malloc : hst\n");
      freefet(nistcom);
      return(-2);
   }
   sprintf(hst, "%s %s %s", fname, class_str, pname);

   /* Build the NISTCOM */
   ret = allocfet_ret(&nistcom, 5);
   if(ret) {
      return(ret);
      free(hst);
   }

   ret = updatefet_ret(NCM_HEADER, "5", nistcom);
   if(ret) {
      freefet(nistcom);
      free(hst);
      return(ret);
   }


   ret = updatefet_ret(NCM_SD_ID, "4", nistcom);
   if(ret) {
      freefet(nistcom);
      free(hst);
      return(ret);
   }

   ret = updatefet_ret(NCM_HISTORY, hst, nistcom);
   if(ret) {
      freefet(nistcom);
      free(hst);
      return(ret);
   }
   free(hst);

   ret = get_sd_class(ihead->id, 4, &class);
   if(ret){
      freefet(nistcom);
      return(ret);
   }
   sprintf(cbuff, "%c", class);
   ret = updatefet_ret(NCM_FING_CLASS, cbuff, nistcom);
   if(ret) {
      freefet(nistcom);
      return(ret);
   }

   ret = updatefet_ret(NCM_SEX, sex, nistcom);
   if(ret) {
      freefet(nistcom);
      return(ret);
   }

   *onistcom = nistcom;
   return(0);
}

/*****************************************/
/* Puts SD9 SD10 SD14 Ihead into NISTCOM */
/*****************************************/
int sd9_10_14_ihead_to_nistcom(NISTCOM **onistcom, IHEAD *ihead,
                               const int sd_id)
{
   char *hst, *fname, *ncic, class, *sex, *pname, *ink_liv;
   char *nptr;
   int ret, hst_sz;
   NISTCOM *nistcom;
   char cbuff[11], id_str[BUFSIZE];

   /* Get information needed from NIST IHEAD Structure */
   strcpy(id_str, ihead->id);
   id_str[12] = '\0';
   fname = id_str;

   id_str[14] = '\0';
   sex = &(id_str[13]);

   id_str[16] = '\0';
   ink_liv = &(id_str[15]);

   ncic = &(id_str[17]);
   nptr = ncic;
   /* put "_" in for " " in the "ncic fpw:??" cases to keep class together */
   while(*nptr != '\0') {
      if(*nptr == ' ')
         *nptr = '_';
      nptr++;
   }

   pname = ihead->parent;

   /* size of HISTORY field "<fname> <ncic> <pname>\0" */
   hst_sz = 2 + strlen(fname) + strlen(ncic) + strlen(pname);

   if((hst = (char *)malloc(hst_sz)) == (char *)NULL){
      fprintf(stderr, "ERROR : sd4_ihead_to_nistcom : malloc : hst\n");
      freefet(nistcom);
      return(-2);
   }
   sprintf(hst, "%s %s %s", fname, ncic, pname);


   /* Build the NISTCOM */
   ret = allocfet_ret(&nistcom, 7);
   if(ret) {
      free(hst);
      return(ret);
   }

   if(sd_id == 14){
      ret = updatefet_ret(NCM_HEADER, "7", nistcom);
      if(ret) {
         freefet(nistcom);
         free(hst);
         return(ret);
      }
      ret = updatefet_ret(NCM_PPI, ihead->density, nistcom);
      if(ret) {
         freefet(nistcom);
         free(hst);
         return(ret);
      }
   }
   else{
      ret = updatefet_ret(NCM_HEADER, "6", nistcom);
      if(ret) {
         freefet(nistcom);
         free(hst);
         return(ret);
      }
   }

   sprintf(cbuff, "%d", sd_id);
   ret = updatefet_ret(NCM_SD_ID, cbuff, nistcom);
   if(ret) {
      freefet(nistcom);
      free(hst);
      return(ret);
   }

   ret = updatefet_ret(NCM_HISTORY, hst, nistcom);
   if(ret) {
      freefet(nistcom);
      free(hst);
      return(ret);
   }
   free(hst);

   ret = get_sd_class(ihead->id, sd_id, &class);
   if(ret){
      freefet(nistcom);
      return(ret);
   }
   sprintf(cbuff, "%c", class);
   ret = updatefet_ret(NCM_FING_CLASS, cbuff, nistcom);
   if(ret) {
      freefet(nistcom);
      return(ret);
   }

   ret = updatefet_ret(NCM_SEX, sex, nistcom);
   if(ret) {
      freefet(nistcom);
      return(ret);
   }

   ret = updatefet_ret(NCM_SCAN_TYPE, ink_liv, nistcom);
   if(ret) {
      freefet(nistcom);
      return(ret);
   }

   *onistcom = nistcom;
   return(0);
}

/********************************/
/* Puts SD18 Ihead into NISTCOM */
/********************************/
int sd18_ihead_to_nistcom(NISTCOM **onistcom, IHEAD *ihead)
{
   char *fname, *sex, *age;
   int ret;
   NISTCOM *nistcom;
   char id_str[BUFSIZE];

   /* Get information needed from NIST IHEAD Structure */
   strcpy(id_str, ihead->id);
   id_str[12] = '\0';
   fname = id_str;

   id_str[14] = '\0';
   sex = &(id_str[13]);

   age = &(id_str[15]);

   /* Build the NISTCOM */
   ret = allocfet_ret(&nistcom, 6);
   if(ret)
      return(ret);

   ret = updatefet_ret(NCM_HEADER, "6", nistcom);
   if(ret) {
      freefet(nistcom);
      return(ret);
   }

   ret = updatefet_ret(NCM_SD_ID, "18", nistcom);
   if(ret) {
      freefet(nistcom);
      return(ret);
   }

   ret = updatefet_ret(NCM_HISTORY, fname, nistcom);
   if(ret) {
      freefet(nistcom);
      return(ret);
   }

   ret = updatefet_ret(NCM_SEX, sex, nistcom);
   if(ret) {
      freefet(nistcom);
      return(ret);
   }

   ret = updatefet_ret(NCM_AGE, age, nistcom);
   if(ret) {
      freefet(nistcom);
      return(ret);
   }

   fname[1] = '\0';
   ret = updatefet_ret(NCM_FACE_POS, fname, nistcom);
   if(ret) {
      freefet(nistcom);
      return(ret);
   }

   *onistcom = nistcom;
   return(0);
}

/*******************************************************************/
int get_sd_class(char *id_str, const int sd_id, char *oclass)
{
   int ret, n, seqnum;
   char class, ncic_str[BUFSIZE];

   if(sd_id == 4){
      if((n = sscanf(id_str, "%*s %c", &class)) <= 0){
         fprintf(stderr, "ERROR : get_sd_class : getting class");
         fprintf(stderr, "letter for Special Database 4\n");
         return(-2);
      }
   }
   else if(sd_id == 10){
      if((n = sscanf(id_str, "%*c%*c%d.%*s %*s %*s %s", &seqnum, ncic_str)) <= 0){
         fprintf(stderr, "ERROR : get_sd_class : getting seqnum and ");
         fprintf(stderr, "ncic classes for Special Database 10\n");
         return(-3);
      }

      ret = get_class_from_ncic_class_string(ncic_str, seqnum, &class);
      if(ret)
         return(ret);
   }
   else if(sd_id == 9 || sd_id == 14){
      if((n = sscanf(id_str, "%*c%d.%*s %*s %*s %s", &seqnum, ncic_str)) <= 0){
         fprintf(stderr, "ERROR : get_sd_class : getting seqnum and ");
         fprintf(stderr, "ncic classes for Special Database 9 or 14\n");
         return(-4);
      }

      ret = get_class_from_ncic_class_string(ncic_str, seqnum, &class);
      if(ret)
         return(ret);
   }
   else{
      fprintf(stderr, "ERROR : get_sd_class : Invalid");
      fprintf(stderr, "database id number (%d)\n", sd_id);
      return(-5);
   }

   *oclass = class;
   return(0);
}

/*******************************************************************/
int get_class_from_ncic_class_string(char *ncic_str, const int seqnum, char *oclass)
{
   char class, *cptr;
   int fing_num, ridge_cnt;

   cptr = ncic_str;

   if(strncmp(cptr, "ac", 2) == 0)
      cptr += 3;

   if(strncmp(cptr, "aa", 2) == 0)
      class = 'A';
   else if(strncmp(cptr, "sr", 2) == 0)
      class = 'S';
   else if(strncmp(cptr, "tt", 2) == 0)
      class = 'T';
   else if(strncmp(cptr, "c", 1) == 0)
      class = 'W';
   else if(strncmp(cptr, "d", 1) == 0)
      class = 'W';
   else if(strncmp(cptr, "p", 1) == 0)
      class = 'W';
   else if(strncmp(cptr, "x", 1) == 0)
      class = 'W';
   else {
      ridge_cnt = atoi(cptr);
      if(ridge_cnt < 1 || ridge_cnt > 99){
         fprintf(stderr, "ERROR : get_class_from_ncic_class_string : ");
         fprintf(stderr, "invalid ridge count (%d) from ncic string\n", ridge_cnt);
         return(-2);
      }

      fing_num = seqnum % 10;
      if((fing_num && fing_num <= 5) == (ridge_cnt < 50))
         class = 'R';
      else
         class = 'L';
   }

   *oclass = class;
   return(0);
}
