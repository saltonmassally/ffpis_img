/***********************************************************************
      LIBRARY: JPEGL - Lossless JPEG Image Compression

      FILE:    ENCODER.C
      AUTHORS: Craig Watson
               Michael Garris
      DATE:    12/01/1997

      Contains routines responsible for JPEGL (lossless) encoding
      image pixel data.

      ROUTINES:
#cat: jpegl_encode_mem - JPEGL encodes image data storing the compressed
#cat:                    bytes to a memory buffer.
#cat: gen_diff_freqs - Computes pixel differences and their fequency.
#cat:
#cat: compress_image_intrlv - Compresses difference values from
#cat:                    non-interleaved image data.
#cat: code_diff - Huffman encodes difference values.
#cat:

***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <jpegl.h>
#include <dataio.h>

/******************/
/*Start of Encoder*/
/******************/
int jpegl_encode_mem(unsigned char **odata, int *olen, IMG_DAT *img_dat,
                     char *comment_text)
{
   int ret, i;
   HUF_TABLE *huf_table[MAX_CMPNTS];
   FRM_HEADER_JPEGL *frm_header;
   JFIF_HEADER *jfif_header;
   unsigned char *outbuf;
   int outlen, outalloc;

   if(debug > 0){
      fprintf(stdout, "Image Data Structure\n");
      fprintf(stdout, "w = %d, h = %d, d = %d, ppi = %d\n",
              img_dat->max_width, img_dat->max_height, img_dat->pix_depth,
              img_dat->ppi);
      fprintf(stdout, "intrlv = %d\n\n", img_dat->intrlv);
      fprintf(stdout, "N = %d\n", img_dat->n_cmpnts);
      for(i = 0; i < img_dat->n_cmpnts; i++)
         fprintf(stdout, "H[%d] = %d, V[%d] = %d\n",
                 i, img_dat->hor_sampfctr[i], i, img_dat->vrt_sampfctr[i]);
      for(i = 0; i < img_dat->n_cmpnts; i++)
         fprintf(stdout, "Pt[%d] = %d, p[%d] = %d\n",
                 i, img_dat->point_trans[i], i, img_dat->predict[i]);
   }

   /* Set output buffer length to size of uncompressed image pixels. */
   outalloc = 0;
   for(i = 0; i < img_dat->n_cmpnts; i++){
      outalloc += (img_dat->samp_width[i] * img_dat->samp_height[i]);
   }

   /* Allocate output buffer. */
   outlen = 0;
   outbuf = (unsigned char *)malloc(outalloc);
   if (outbuf == (unsigned char *)NULL){
      fprintf(stderr, "ERROR : jpegl_encode_mem : malloc : outbuf\n");
      return(-2);
   }

   ret = putc_ushort(SOI, outbuf, outalloc, &outlen);
   if(ret){
      free(outbuf);
      return(ret);
   }

   ret = setup_jfif_header(&jfif_header, PPI_UNITS,
		   img_dat->ppi, img_dat->ppi);
   if(ret){
      free(outbuf);
      return(ret);
   }

   ret = putc_jfif_header(jfif_header, outbuf, outalloc, &outlen);
   if(ret){
      free(outbuf);
      free(jfif_header);
      return(ret);
   }
   free(jfif_header);

   ret = putc_nistcom_jpegl(comment_text, img_dat->max_width,
		   img_dat->max_height, img_dat->pix_depth, img_dat->ppi,
		   0 /* lossless */, img_dat->n_cmpnts, img_dat->hor_sampfctr,
		   img_dat->vrt_sampfctr, img_dat->predict[0],
		   outbuf, outalloc, &outlen);
   if(ret){
      free(outbuf);
      return(ret);
   }

   ret = setup_frame_header_jpegl(&frm_header, img_dat);
   if(ret){
      free(outbuf);
      return(ret);
   }

   ret = putc_frame_header_jpegl(frm_header, outbuf, outalloc, &outlen);
   if(ret){
      free(outbuf);
      free(frm_header);
      return(ret);
   }
   free(frm_header);

   ret = gen_diff_freqs(img_dat, huf_table);
   if(ret){
      free(outbuf);
      return(ret);
   }

   ret = gen_huff_tables(huf_table, img_dat->n_cmpnts);
   if(ret){
      free(outbuf);
      free_HUFF_TABLES(huf_table, img_dat->n_cmpnts);
      return(ret);
   }

   ret = compress_image_non_intrlv(img_dat, huf_table,
		   outbuf, outalloc, &outlen);
   if(ret){
      free(outbuf);
      free_HUFF_TABLES(huf_table, img_dat->n_cmpnts);
      return(ret);
   }
   free_HUFF_TABLES(huf_table, img_dat->n_cmpnts);

   ret = putc_ushort(EOI, outbuf, outalloc, &outlen);
   if(ret){
      free(outbuf);
      return(ret);
   }

   *odata = outbuf;
   *olen = outlen;
   return(0);
}

/*****************************************/
/*routine to obtain the pixel differences*/
/*****************************************/
int gen_diff_freqs(IMG_DAT *img_dat, HUF_TABLE **huf_table)
{
   int ret, i, pixel, np; /*current pixel and total number of pixels*/
   short data_pred;       /*predicted pixel value*/
   short *data_diff;      /*difference values*/
   short diff_cat;        /*difference category*/
   unsigned char *indata;
   unsigned char p, Pt;

   /* Need this initialization for deallocation of huf_table upon ERROR. */
   for(i = 0; i < img_dat->n_cmpnts; i++)
      huf_table[i] = (HUF_TABLE *)NULL;

   /* Foreach component ... */
   for(i = 0; i < img_dat->n_cmpnts; i++) {
      np = (img_dat->samp_width[i] * img_dat->samp_height[i]);
      /* Calloc inits all member addresses to NULL. */
      huf_table[i] = (HUF_TABLE *)calloc(1, sizeof(HUF_TABLE));
      if(huf_table[i] == (HUF_TABLE *)NULL){
         fprintf(stderr, "ERROR : gen_diff_freqs : calloc : ");
         fprintf(stderr, "huf_table[%d]\n", i);
         free_HUFF_TABLES(huf_table, i);
         return(-2);
      }
      huf_table[i]->freq = (int *)calloc(MAX_HUFFCOUNTS_JPEGL+1, sizeof(int));
      if(huf_table[i]->freq == (int *)NULL){
         fprintf(stderr, "ERROR : gen_diff_freqs : calloc : ");
         fprintf(stderr, "huf_table[%d]->freq\n", i);
         free_HUFF_TABLES(huf_table, i+1);
         return(-3);
      }

      huf_table[i]->freq[MAX_HUFFCOUNTS_JPEGL] = 1;

      img_dat->diff[i] = (short *)malloc(np * sizeof(short));
      if(img_dat->diff[i] == (short *)NULL){
         fprintf(stderr, "ERROR : gen_diff_freqs : malloc : ");
         fprintf(stderr, "img_dat->diff[%d]\n", i);
         free_HUFF_TABLES(huf_table, i+1);
         return(-4);
      }

      /* If intrlv ... */
      if(!(img_dat->intrlv)) {
         Pt = img_dat->point_trans[i];
         p = img_dat->predict[i];
      }
      /* Otherwise, nonintrlv ... */
      else {
         Pt = img_dat->point_trans[0];
         p = img_dat->predict[0];
      }

      /* Set pointer to next component plane origin. */
      indata = img_dat->image[i];
      data_diff = img_dat->diff[i];
      for(pixel = 0; pixel < np; pixel++) {
         *indata >>= Pt;
	 ret = predict(&data_pred, indata, img_dat->samp_width[i],
			 pixel, img_dat->cmpnt_depth, p, Pt);
         if(ret){
            free_HUFF_TABLES(huf_table, i+1);
            return(ret);
         }
         *data_diff = ((short)*indata) - data_pred;
         indata++;
         diff_cat = categorize(*data_diff);
         if((diff_cat < 0) || (diff_cat > MAX_HUFFCOUNTS_JPEGL)){
            fprintf(stderr, "ERROR : gen_diff_freqs : ");
            fprintf(stderr, "Invalid code length = %d\n", diff_cat);
            free_HUFF_TABLES(huf_table, i+1);
            return(-5);
         }
         huf_table[i]->freq[diff_cat]++;
         data_diff++;
      }

      if(debug > 2){
         for(pixel = 0; pixel < MAX_HUFFCOUNTS_JPEGL+1; pixel++)
            fprintf(stdout, "freqs[%d] = %d\n", pixel,
                    huf_table[i]->freq[pixel]);
      }
   }

   return(0);
}

/*********************************************/
/*Routine to "compress" the difference values*/
/*********************************************/
int compress_image_non_intrlv(IMG_DAT *img_dat, HUF_TABLE **huf_table,
             unsigned char *outbuf, const int outalloc, int *outlen)
{
   int ret, size;       /*huffman code size*/
   unsigned int code;           /*huffman code*/
   int i, i2, p, np; /*current pixel and total number of pixels*/
   unsigned char bits;          /*bits to transfer*/
   HUFFCODE *huff_encoder;
   short *diffptr;
   int outbit = FIRSTBIT;
   SCN_HEADER *scn_header;
   unsigned char *outptr;


   for(i = 0; i < img_dat->n_cmpnts; i++) {
      ret = putc_huffman_table(DHT, huf_table[i]->table_id,
		      huf_table[i]->bits, huf_table[i]->values, outbuf,
		      outalloc, outlen);
      if(ret)
         return(ret);

      ret = setup_scan_header(&scn_header, img_dat, i);
      if(ret)
         return(ret);

      ret = putc_scan_header(scn_header, outbuf, outalloc, outlen);
      if(ret)
         return(ret);
      free(scn_header);

      huff_encoder = (HUFFCODE *)calloc((LARGESTDIFF<<1)+1,
                                        sizeof(HUFFCODE));
      if(huff_encoder == (HUFFCODE *)NULL){
         fprintf(stderr, "ERROR : compress_image_non_intrlv : ");
         fprintf(stderr, "calloc : huff_encoder[%d]\n", i);
         return(-2);
      }

      np =(img_dat->samp_width[i] * img_dat->samp_height[i]);
      diffptr = img_dat->diff[i];

      if((*outlen) >= outalloc){
         fprintf(stderr, "ERROR : compress_image_non_intrlv : ");
         fprintf(stderr, "buffer overlow: alloc = %d, request = %d\n",
                          outalloc, *outlen);
         free(huff_encoder);
         return(-3);
      }
      outptr = outbuf + (*outlen);
      *outptr = 0;

      for(p = 0; p < np; p++) {
         ret = code_diff(huf_table[i]->huffcode_table,
			 (huff_encoder + (*diffptr) + LARGESTDIFF),
			 &size, &code, diffptr);
         if(ret){
            free(huff_encoder);
            return(ret);
         }

         diffptr++;

         for(--size; size >= 0; size--) {
            bits = ((unsigned char)((code >> size) & LSBITMASK));
            if((bits & BITSET) != 0)
               *outptr |= (BITSET << outbit);

            if(--(outbit) < 0) {
               if(*outptr == 0xff) {
                  (*outlen)++;
                  outptr++;
                  if((*outlen) >= outalloc){
                     fprintf(stderr, "ERROR : compress_image_intrlv : ");
                     fprintf(stderr, "buffer overlow: ");
                     fprintf(stderr, "alloc = %d, request = %d\n",
                                      outalloc, *outlen);
                     free(huff_encoder);
                     return(-4);
                  }
                  *outptr = 0;
               }
               (*outlen)++;
               outptr++;
               if((*outlen) >= outalloc){
                  fprintf(stderr, "ERROR : compress_image_intrlv : ");
                  fprintf(stderr, "buffer overlow: ");
                  fprintf(stderr, "alloc = %d, request = %d\n",
                                   outalloc, *outlen);
                  free(huff_encoder);
                  return(-5);
               }
               *outptr = 0;
               outbit = FIRSTBIT;
            }
         }
      }
      free(huff_encoder);

      /* Flush the Buffer */
      if(outbit != FIRSTBIT) {
         for(i2 = outbit; i2 >= 0; i2--)
            *outptr |= (BITSET << i2);
         if(*outptr == 0xff) {
            (*outlen)++;
            outptr++;
            if((*outlen) >= outalloc){
               fprintf(stderr, "ERROR : compress_image_non_intrlv : ");
               fprintf(stderr, "buffer overlow: ");
               fprintf(stderr, "alloc = %d, request = %d\n",
                                outalloc, *outlen);
               return(-6);
            }
            *outptr = 0;
         }
         (*outlen)++;
      }
      outbit = FIRSTBIT;
   }

   return(0);
}

/*****************************************************************/
/*Routine to build code table and code existing difference values*/
/*****************************************************************/
int code_diff(HUFFCODE *huffcode_table, HUFFCODE *huff_encoder,
              int *new_size, unsigned int *new_code, short *pdiff)
{

   int nextbit, shift, i, cat;      /*variables used to obtain desired
					codes and sizes*/
   short diff;

   diff = *pdiff;

   if((huff_encoder->size) == 0) {
      cat = (int)categorize(diff);
      if((cat < 0) || (cat > MAX_HUFFCOUNTS_JPEGL)){
	 fprintf(stderr, "ERROR : code_diff : invalid code length = %d\n",
                 cat);
         return(-2);
      }
      *new_size = (huffcode_table + cat)->size;
      nextbit = (MAX_HUFFBITS<<1) - *new_size;
      *new_code = ((huffcode_table + cat)->code) << (nextbit);
      nextbit--;
      if(diff < 0)
	 diff--;
      shift = cat - 1;
      for(i = 0; i < cat; i++) {
	 if(((diff >> shift) & LSBITMASK) != 0)
	    *new_code |= (LSBITMASK << (nextbit));
	 shift--;
	 nextbit--;
      }
      *new_size += cat;
      *new_code >>= ((MAX_HUFFBITS<<1) - *new_size);
      huff_encoder->size = *new_size;
      huff_encoder->code = *new_code;
   }
   else {
      *new_code = huff_encoder->code;
      *new_size = huff_encoder->size;
   }

   return(0);
}
