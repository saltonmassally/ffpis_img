/***********************************************************************
      LIBRARY: IHEAD - IHead Image Utilities

      FILE:    PRNTIHDR.C
      AUTHORS: Michael Garris
      DATE:    04/26/1989

      Contains routines responsible for printing the contents of
      an IHead header.

      ROUTINES:
#cat: printihdr - formats the attributes of a specified IHead structure
#cat:             into a report and prints it to an open file pointer.

***********************************************************************/

#include <stdio.h>
#include <ihead.h>

/************************************************************/
/* Printihdr() prints the contents of an ihead structure to */
/* the passed file pointer.                                 */
/************************************************************/
void printihdr( IHEAD *head, FILE *fp)
{

   fprintf(fp,"IMAGE FILE HEADER\n");
   fprintf(fp,"~~~~~~~~~~~~~~~~~\n");
   fprintf(fp,"Identity\t:  %s\n", head->id);
   fprintf(fp,"Header Size\t:  %d (bytes)\n",sizeof(IHEAD));
   fprintf(fp,"Date Created\t:  %s\n", head->created);
   fprintf(fp,"Width\t\t:  %s (pixels)\n",head->width);
   fprintf(fp,"Height\t\t:  %s (pixels)\n",head->height);
   fprintf(fp,"Bits per Pixel\t:  %s\n",head->depth);
   fprintf(fp,"Resolution\t:  %s (ppi)\n",head->density);
   fprintf(fp,"Compression\t:  %s (code)\n",head->compress);
   fprintf(fp,"Compress Length\t:  %s (bytes)\n",head->complen);
   fprintf(fp,"Scan Alignment\t:  %s (bits)\n",head->align);
   fprintf(fp,"Image Data Unit\t:  %s (bits)\n",head->unitsize);
   if(head->byte_order == HILOW)
      fprintf(fp,"Byte Order\t:  High-Low\n");
   else
      fprintf(fp,"Byte Order\t:  Low-High\n");
   if(head->sigbit == MSBF)
      fprintf(fp,"MSBit\t\t:  First\n");
   else
      fprintf(fp,"MSBit\t\t:  Last\n");
   fprintf(fp,"Column Offset\t:  %s (pixels)\n",head->pix_offset);
   fprintf(fp,"White Pixel\t:  %s\n",head->whitepix);
   if(head->issigned == SIGNED)
      fprintf(fp,"Data Units\t:  Signed\n");
   else
      fprintf(fp,"Data Units\t:  Unsigned\n");
   fprintf(fp,"Scan Order\t:  ");
   if(head->rm_cm == ROW_MAJ)
      fprintf(fp,"Row Major,\n");
   else
      fprintf(fp,"Column Major,\n");
   if(head->tb_bt == TOP2BOT)
      fprintf(fp,"\t\t   Top to Bottom,\n");
   else
      fprintf(fp,"\t\t   Bottom to Top,\n");
   if(head->lr_rl == LEFT2RIGHT)
      fprintf(fp,"\t\t   Left to Right\n");
   else
      fprintf(fp,"\t\t   Right to Left\n");
   /* EDIT MDG 1/25/99
   if(*(head->parent) != NULL){
   */
   if(*(head->parent) != '\0'){
      fprintf(fp,"Parent\t\t:  %s\n",head->parent);
      fprintf(fp,"X Origin\t:  %s (pixels)\n",head->par_x);
      fprintf(fp,"Y Origin\t:  %s (pixels)\n",head->par_y);
   }
}

