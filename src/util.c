/***********************************************************************
      LIBRARY: JPEGL - Lossless JPEG Image Compression

      FILE:    UTIL.C
      AUTHORS: Craig Watson
               Michael Garris
      DATE:    11/28/2000

      Contains general routines responsible for supporting JPEGL
      (lossless) image compression.

      ROUTINES:
#cat: predict - Used to predict the pixel values in an image.
#cat:
#cat: categorize - Determines the category for a given difference value.
#cat:

***********************************************************************/

#include <stdio.h>
#include <jpegl.h>

/**************************************************************/
/*This routine is used to predict the pixel values in an image*/
/**************************************************************/
int predict(short *odata_pred, unsigned char *indata, const int width,
            const int pixel_num, const int cmpnt_depth, const int pred_type,
            const int Pt)
{
   short data_pred;                     /*pixel prediction*/

   if(pixel_num == 0) {                 /*determine predictor for first pixel*/
      data_pred = (1 << (cmpnt_depth-Pt-1));
      *odata_pred = data_pred;
      return(0);
   }

   else if(pixel_num > 0 && pixel_num < width) {
      data_pred = *(indata - 1);        /*determine predictor for first
                                          line in the non-interleaved
                                          component*/
      *odata_pred = data_pred;
      return(0);
   }
                                        /*determine predictor for the
                                          rest of the pixels in the 
                                          component*/
   else if(pixel_num > (width - 1))  {
      if((pixel_num % width) == 0)      /*predictor if pixel is at the
                                          beginning of a line*/
	 data_pred = *(indata - width);
      else{ 
         switch(pred_type) {            /*various predictor types defined
                                          in the standard*/
            case PRED1:
               data_pred = *(indata - 1);
               break;
            case PRED2:
               data_pred = *(indata - width);
               break;
            case PRED3:
               data_pred = *(indata -(width + 1));
               break;
            case PRED4:
               data_pred = *(indata - 1) + 
                           *(indata - width) -
                           *(indata -(width + 1));
               break;
            case PRED5:
               data_pred = *(indata - 1) + 
                           ((*(indata - width) >> 1) - 
                           (*(indata -(width + 1)) >> 1));
               break;
            case PRED6:
               data_pred = *(indata - width) + 
                           ((*(indata - 1) >> 1) - 
                           (*(indata -(width + 1)) >> 1));
               break;
            case PRED7:
               data_pred = (*(indata -1) + *(indata - width)) / 2;
               break;
            default:
               fprintf(stderr, "ERORR : predict : invalid prediction type ");
               fprintf(stderr, "%d not in range [%d..%d]\n",
                                pred_type, PRED1, PRED7);
               return(-2);
         }
      }
   }
   else { data_pred=128;}

   *odata_pred = data_pred;
   return(0);
}

/********************************************************************/
/*This function determines the category for a given difference value*/
/********************************************************************/
short categorize(const short idiff)
{
   int bit;                     /*bit pointer to difference value*/
   short diff;

   diff = idiff;

   if(diff == 0)
      return (0);               /*difference category zero*/

   if (diff < 0)
      diff *= -1;
   
   for (bit = 0; bit < MAX_HUFFBITS; bit++) {
      if ((diff & CATMASK) != 0)
	 return ((short)(MAX_HUFFBITS - bit));
      diff <<= 1;
   }
   return(-1);
}
