/***********************************************************************
      LIBRARY: IHEAD - IHead Image Utilities

      FILE:    NULLIHDR.C
      AUTHOR:  Michael Garris
      DATE:    05/01/1989

      Contains routines responsible for initializing an IHead header
      to be empty.

      ROUTINES:
#cat: nullihdr - initializes an allocated IHead structure to be empty.
#cat:

***********************************************************************/

#include <ihead.h>

/************************************************************/
/* Nullihdr() nulls all bytes in a ihead structure.         */
/************************************************************/
void nullihdr( IHEAD *head)
{
   int i;

   for(i = 0; i < BUFSIZE; i++){
      (head->id)[i] = '\0';
      (head->parent)[i] = '\0';
   }
   for(i = 0; i < DATELEN; i++)
      (head->created)[i] = '\0';
   for(i = 0; i < SHORT_CHARS; i++){
      (head->width)[i] = '\0';
      (head->height)[i] = '\0';
      (head->depth)[i] = '\0';
      (head->density)[i] = '\0';
      (head->compress)[i] = '\0';
      (head->complen)[i] = '\0';
      (head->align)[i] = '\0';
      (head->unitsize)[i] = '\0';
      (head->pix_offset)[i] = '\0';
      (head->whitepix)[i] = '\0';
      (head->par_x)[i] = '\0';
      (head->par_y)[i] = '\0';
   }
   head->sigbit = '\0';
   head->byte_order = '\0';
   head->issigned = '\0';
   head->rm_cm = '\0';
   head->tb_bt = '\0';
   head->lr_rl = '\0';
}

