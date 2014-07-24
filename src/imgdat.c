/***********************************************************************
      LIBRARY: JPEGL - Lossless JPEG Image Compression

      FILE:    IMGDAT.C
      AUTHORS: Craig Watson
               Michael Garris
      DATE:    11/28/2000

      Contains routines responsible for manipulating an IMG_DAT
      (image data) structure that hold an image's pixmap and
      related attributes.

      ROUTINES:
#cat: get_IMG_DAT_image - Extracts the image pixmap and returns its
#cat:                     attributes stored in an IMG_DAT structure.
#cat: setup_IMG_DAT_nonintrlv_encode - Initialize an IMG_DAT structure
#cat:                     for JPEGL compressing a non-interleaved pixmap.
#cat: setup_IMG_DAT_decode - Initialize an IMG_DAT structure for
#cat:                     compressing a general pixmap.
#cat: update_IMG_DAT_decode - Augments an IMG_DAT structure used for
#cat:                     decompression with attributes derived from a
#cat:                     JPEGL SCN Header, including the allocation for
#cat:                     the reconstructred pixmap.
#cat: free_IMG_DAT - Deallocates an IMG_DAT structure.

***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <jpegl.h>

/**************************************/
/* Extract image from image structure */
/**************************************/
int get_IMG_DAT_image(unsigned char **odata, int *olen,
                      int *width, int *height, int *depth, int *ppi,
                      IMG_DAT *img_dat)
{
   int i, nsizes[MAX_CMPNTS], nlen;
   unsigned char *ndata, *nptr;

   nlen = 0;
   for(i = 0; i < img_dat->n_cmpnts; i++){
      nsizes[i] = img_dat->samp_width[i] * img_dat->samp_height[i];
      nlen += nsizes[i];
   }

   ndata = (unsigned char *)malloc(nlen * sizeof(unsigned char));
   if(ndata == (unsigned char *)NULL){
      fprintf(stderr, "ERROR : get_IMG_DAT_image : malloc : ndata\n");
      return(-2);
   }

   nptr = ndata;
   for(i = 0; i < img_dat->n_cmpnts; i++){
      memcpy(nptr, img_dat->image[i], nsizes[i] * sizeof(unsigned char));
      nptr += nsizes[i];
   }

   *odata = ndata;
   *olen = nlen;
   *width = img_dat->max_width;
   *height = img_dat->max_height;
   *depth = img_dat->pix_depth;
   *ppi = img_dat->ppi;

   return(0);
}

/*******************************************/
/* Setup image structure to compress image */
/*******************************************/
int setup_IMG_DAT_nonintrlv_encode(IMG_DAT **oimg_dat, unsigned char *idata,
          const int w, const int h, const int d, const int ppi,
          int *hor_sampfctr, int *vrt_sampfctr, const int n_cmpnts,
          const unsigned char pt_val, const unsigned char pred_val)
{
   int i, j, max_hor, max_vrt, plane_size;
   IMG_DAT *img_dat;
   unsigned char *iptr;

   if((d != 8) && (d != 24)){
      fprintf(stderr, "ERROR : setup_IMG_DAT_nonintrlv_encode : ");
      fprintf(stderr, "image pixel depth %d != 8 or 24\n", d);
      return(-2);
   }

   if(n_cmpnts > MAX_CMPNTS){
      fprintf(stderr, "ERROR : setup_IMG_DAT_nonintrlv_encode : ");
      fprintf(stderr, "number of components = %d > %d\n",
              n_cmpnts, MAX_CMPNTS);
      return(-3);
   }

   if(((d == 8) && (n_cmpnts != 1)) ||
      ((d == 24) && (n_cmpnts != 3))){
      fprintf(stderr, "ERROR : setup_IMG_DAT_nonintrlv_encode : ");
      fprintf(stderr, "depth = %d mismatched with n_cmpnts = %d\n",
              d, n_cmpnts);
      return(-4);
   }

   if((img_dat = (IMG_DAT *)calloc(1, sizeof(IMG_DAT))) == (IMG_DAT *)NULL){
      fprintf(stderr, "ERROR : setup_IMG_DAT_nonintrlv_encode : ");
      fprintf(stderr, "calloc : img_dat\n");
      return(-5);
   }

   img_dat->max_width = w;
   img_dat->max_height = h;
   img_dat->pix_depth = d;
   img_dat->ppi = ppi;
   img_dat->intrlv = NO_INTRLV;
   img_dat->n_cmpnts = n_cmpnts;
   img_dat->cmpnt_depth = 8;  /* data units must be unsigned char */

   /* Determine max tile dimensions across all components ... */
   max_hor = -1;
   max_vrt = -1;
   for(i = 0; i < n_cmpnts; i++){
      if(hor_sampfctr[i] > max_hor)
         max_hor = hor_sampfctr[i];
      if(vrt_sampfctr[i] > max_vrt)
         max_vrt = vrt_sampfctr[i];
   }

   iptr = idata;
   for(i = 0; i < n_cmpnts; i++){
      img_dat->hor_sampfctr[i] = hor_sampfctr[i];
      img_dat->vrt_sampfctr[i] = vrt_sampfctr[i];
      /* Compute the pixel width & height of the component's plane.  */
      img_dat->samp_width[i] = ceil(img_dat->max_width *
                                         (hor_sampfctr[i] / (double)max_hor));
      img_dat->samp_height[i] = ceil(img_dat->max_height *
                                          (vrt_sampfctr[i] / (double)max_vrt));
      img_dat->point_trans[i] = pt_val;
      img_dat->predict[i] = pred_val;

      plane_size = img_dat->samp_width[i] * img_dat->samp_height[i];
      img_dat->image[i] =
               (unsigned char *)malloc(plane_size * sizeof(unsigned char));
      if(img_dat->image[i] == (unsigned char *)NULL){
         fprintf(stderr, "ERROR : setup_IMG_DAT_nonintrlv_encode : ");
         fprintf(stderr, "malloc : img_dat->image[%d]\n", i);
         for(j = 0; j < i; j++)
            free(img_dat->image[j]);
         free(img_dat);
         return(-6);
      }
      memcpy(img_dat->image[i], iptr, plane_size);

      /* Bump to start of next component plane. */
      iptr += plane_size;
   }

   *oimg_dat = img_dat;
   return(0);
}

/*********************************************/
int setup_IMG_DAT_decode(IMG_DAT **oimg_dat, const int ppi,
                         FRM_HEADER_JPEGL *frm_header)
{
   int i, max_hor, max_vrt;
   IMG_DAT *img_dat;

   img_dat = (IMG_DAT *)calloc(1, sizeof(IMG_DAT));
   if(img_dat == (IMG_DAT *)NULL){
      fprintf(stderr, "ERROR : setup_IMG_DAT_decode : calloc : img_dat\n");
      return(-2);
   }

   img_dat->max_width = frm_header->x;
   img_dat->max_height = frm_header->y;
   img_dat->pix_depth = frm_header->Nf * 8;
   img_dat->ppi = ppi;
   img_dat->intrlv = -1;
   img_dat->n_cmpnts = frm_header->Nf;
   img_dat->cmpnt_depth = frm_header->prec;

   max_hor = -1;   
   max_vrt = -1;   
   for(i = 0; i < img_dat->n_cmpnts; i++){
      img_dat->hor_sampfctr[i] = frm_header->HV[i]>>4;
      img_dat->vrt_sampfctr[i] = frm_header->HV[i] & 0x0F;
      if(max_hor < img_dat->hor_sampfctr[i])
         max_hor = img_dat->hor_sampfctr[i];
      if(max_vrt < img_dat->vrt_sampfctr[i])
         max_vrt = img_dat->vrt_sampfctr[i];
   }

   for(i = 0; i < img_dat->n_cmpnts; i++){
      img_dat->samp_width[i] = ceil(img_dat->max_width *
                               (img_dat->hor_sampfctr[i]/(double)max_hor));
      img_dat->samp_height[i] = ceil(img_dat->max_height *
                               (img_dat->vrt_sampfctr[i]/(double)max_vrt));
   }

   *oimg_dat = img_dat;
   return(0);
}

/*********************************************/
int update_IMG_DAT_decode(IMG_DAT *img_dat, SCN_HEADER *scn_header,
                          HUF_TABLE **huf_table)
{
   int i, cmpnt_i;

   if(scn_header->Ns > 1)
      img_dat->intrlv = 1;
   else
      img_dat->intrlv = 0;

   /* NOTE: scn_header->Ns == 1 if encoded data is NOT interleaved. */
   for(i = 0; i < scn_header->Ns; i++) {
      cmpnt_i = scn_header->Cs[i];
      if((huf_table[cmpnt_i] == (HUF_TABLE *)NULL) ||
         (huf_table[cmpnt_i]->def != 1)){
         fprintf(stderr, "ERROR : update_IMG_DAT_decode : ");
         fprintf(stderr, "huffman table %d not defined\n", cmpnt_i);
         return(-2);
      }
      img_dat->point_trans[cmpnt_i] = scn_header->Ahl;
      img_dat->predict[cmpnt_i] = scn_header->Ss;
      img_dat->image[cmpnt_i] =
               (unsigned char *)malloc(img_dat->samp_width[cmpnt_i] *
                                       img_dat->samp_height[cmpnt_i]);
      if(img_dat->image[cmpnt_i] == (unsigned char *)NULL){
         fprintf(stderr, "ERROR : update_IMG_DAT_decode : ");
         fprintf(stderr, "malloc : img_dat->image[%d]\n", cmpnt_i);
         return(-3);
      }
   }

   return(0);
}

/*********************************************/
#if 0
int setup_IMG_DAT_decode_old(IMG_DAT **oimg_dat, const int ppi,
                         FRM_HEADER_JPEGL *frm_header,
                         SCN_HEADER *scn_header, HUF_TABLE **huf_table)
{
   int i, cmpnt_i;
   IMG_DAT *img_dat;

   img_dat = (IMG_DAT *)calloc(1, sizeof(IMG_DAT));
   if(img_dat == (IMG_DAT *)NULL){
      fprintf(stderr, "ERROR : setup_IMG_DAT_decode : calloc : img_dat\n");
      return(-2);
   }

   img_dat->n_cmpnts = frm_header->Nf;
   if(scn_header->Ns > 1)
      img_dat->intrlv = 1;
   else
      img_dat->intrlv = 0;

   if(!(img_dat->intrlv)) {
      cmpnt_i = scn_header->Cs[0];
      if((huf_table[cmpnt_i] == (HUF_TABLE *)NULL) ||
         (huf_table[cmpnt_i]->def != 1)){
         fprintf(stderr, "ERROR : setup_IMG_DAT_decode : ");
         fprintf(stderr, "huffman table %d not defined %d\n", cmpnt_i,
                    scn_header->Ns);
         free_IMG_DAT(img_dat, NO_FREE_IMAGE);
	 return(-3);
      }
      img_dat->point_trans[cmpnt_i] = scn_header->Ahl;
      img_dat->predict[cmpnt_i] = scn_header->Ss;
      img_dat->max_width = frm_header->x;
      img_dat->max_height = frm_header->y;
      img_dat->pix_depth = frm_header->prec;
      img_dat->ppi = ppi;

      img_dat->image[cmpnt_i] = (unsigned char *)malloc(img_dat->max_width *
                                          img_dat->max_height);
      if(img_dat->image[cmpnt_i] == (unsigned char *)NULL){
         fprintf(stderr, "ERROR : setup_IMG_DAT_decode : ");
         fprintf(stderr, "malloc : img_dat->image[%d]\n", cmpnt_i);
         free_IMG_DAT(img_dat, NO_FREE_IMAGE);
	 return(-4);
      }
   }
   else {
      img_dat->max_width = frm_header->x;
      img_dat->max_height = frm_header->y;
      img_dat->pix_depth = frm_header->prec;
      img_dat->ppi = ppi;

      for(i = 0; i < scn_header->Ns; i++) {
	 cmpnt_i = scn_header->Cs[i];
         if((huf_table[cmpnt_i] == (HUF_TABLE *)NULL) ||
            (huf_table[cmpnt_i]->def != 1)){
            fprintf(stderr, "ERROR : setup_IMG_DAT_decode : ");
            fprintf(stderr, "huffman table %d not defined\n", cmpnt_i);
            free_IMG_DAT(img_dat, NO_FREE_IMAGE);
	    return(-5);
         }
	 img_dat->point_trans[cmpnt_i] = scn_header->Ahl;
	 img_dat->predict[cmpnt_i] = scn_header->Ss;

         img_dat->image[cmpnt_i] = (unsigned char *)malloc(img_dat->max_width *
                                             img_dat->max_height);
         if(img_dat->image[cmpnt_i] == (unsigned char *)NULL){
            fprintf(stderr, "ERROR : setup_IMG_DAT_decode : ");
            fprintf(stderr, "malloc : img_dat->image[%d]\n", cmpnt_i);
            free_IMG_DAT(img_dat, NO_FREE_IMAGE);
	    return(-6);
         }
      }
   }

   *oimg_dat = img_dat;
   return(0);
}
#endif


/******************************/
/* Deallocate image structure */
/******************************/
void free_IMG_DAT(IMG_DAT *img_dat, const int img_flag)
{
   int i;

   for(i = 0; i < img_dat->n_cmpnts; i++){
      if(img_dat->diff[i] != (short *)NULL)
         free(img_dat->diff[i]);
   }

   if(img_flag){
      for(i = 0; i < img_dat->n_cmpnts; i++){
	 if(img_dat->image[i] != (unsigned char *)NULL)
            free(img_dat->image[i]);
      }
   }

   free(img_dat);
}
