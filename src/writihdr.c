/***********************************************************************
      LIBRARY: IHEAD - IHead Image Utilities

      FILE:    WRITIHDR.C
      AUTHOR:  Michael Garris
      DATE:    02/08/1990

      Contains routines responsible for writing the contents
      of an IHead header to an open file.

      ROUTINES:
#cat: writeihdr - writes the contents of an IHead structure to an open
#cat:             file pointer.

***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ihead.h>
#include <sys/param.h>
#include <ffpis/util/util.h>
#include <time.h>
#include <ffpis/util/little.h>
#include "imgutil.h"
#include "imgdecod.h"
#include "ioutil.h"
#include "getnset.h"
#include "memalloc.h"
#include "grp4comp.h"


/************************************************************/
/* Writeihdr() writes the fixed length field and the header */
/* passed to the given file pointer.                        */
/************************************************************/
void writeihdr( FILE *fp, IHEAD *ihead)
{
   int i;
   char lenstr[SHORT_CHARS];

   for (i = 0; i < SHORT_CHARS; i++)
      lenstr[i] = '\0';
   /* creates a string from of header size */
   sprintf(lenstr,"%d",sizeof(IHEAD));
   /* writes the length string in headerto the file */
   fwrite(lenstr,sizeof(char),SHORT_CHARS,fp);
   /* writes the given header to the file */
   fwrite(ihead,sizeof(IHEAD),1,fp);
}
/***********************************************************************
      LIBRARY: IMAGE - Image Manipulation and Processing Routines

      FILE:    WRITIHDR.C

      AUTHOR:  Michael Garris
      DATE:    04/26/1989

      Contains routines responsible for encoding/writing an
      IHead image file.

      ROUTINES:
#cat: writeihdrfile - writes the contents of an IHead structure and an
#cat:                 image memory to the specified file.
#cat: writeihdrsubimage - generates an IHead based on the parameters passed
#cat:                     and writes a subimage to the specified file.
#cat: write_fields - takes a list of IHead structures and binary images
#cat:                and writes them to individual files with the specified
#cat:                root filename and corresponding extension name.

***********************************************************************/

/************************************************************/
/*         Routine:   Writeihdrfile()                       */
/*         Author:    Michael D. Garris                     */
/*         Date:      4/26/89                               */
/*         Modifications:                                   */
/*           9/20/90   (Stan Janet) check return codes      */
/*           2/20/91   (MDG) compression capability         */
/*   Modified 12/94 by Patrick Grother                         */
/*         Allocate 4 times the original image for the rare    */
/*         case (noisy image) where the compressed result has  */
/*         grown in size to be larger than the original image  */

/************************************************************/
/************************************************************/
/* Writeihdrfile() writes a ihead structure and correspon-  */
/* ding image data to the specified file.                   */
/************************************************************/
void writeihdrfile(
char *file,
IHEAD *head,
unsigned char *data)
{
   FILE *fp;
   int width,height,depth,code,filesize,n, compbytes;
   unsigned char *compdata;

   /* reopen the image file for writing */
   fp = fopen(file,"wb");
   if (fp == NULL)
      syserr("writeihdrfile","fopen",file);

   n = sscanf(head->width,"%d",&width);
   if (n != 1)
      fatalerr("writeihdrfile","sscanf failed on width field",NULL);
   n = sscanf(head->height,"%d",&height);
   if (n != 1)
      fatalerr("writeihdrfile","sscanf failed on height field",NULL);
   n = sscanf(head->depth,"%d",&depth);
   if (n != 1)
      fatalerr("writeihdrfile","sscanf failed on depth field",NULL);
   n = sscanf(head->compress, "%d", &code);
   if (n != 1)
      fatalerr("writeihdrfile","sscanf failed on compression code field",NULL);

   filesize = SizeFromDepth(width,height,depth);
   if ( code == CCITT_G4 )  /* G4 compression breaks for noisy images since */
      filesize *= 4;        /* the result is larger than the original. The  */
                            /* buffer is exceeded and a segmentation fault  */
                            /* occurs. The factor 4 is empirically found    */
                            /* to be sufficient.                            */

   if(code == UNCOMP){
      sprintf(head->complen, "%d", 0);
      writeihdr(fp,head);
      n = fwrite(data,1,filesize,fp);
      if (n != filesize)
         syserr("writeihdrfile", "fwrite", file);
   }
   else{
      malloc_uchar(&compdata, filesize, "writeihdrfile : compdata");
      switch(code){
      case CCITT_G3:
         fatalerr("writeihdrfile","G3 compression not implemented.",NULL);
         break;
      case CCITT_G4:
         if(depth != 1)
            fatalerr("writeihdrfile",
                     "G4 compression requires a binary image.", NULL);
         grp4comp(data, filesize, width, height, compdata, &compbytes);
         break;
      case RL:
         rlcomp(data, filesize, compdata, &compbytes, filesize);
         break;
      default:
         fatalerr("writeihdrfile","Unknown compression",NULL);
         break;
      }
      sprintf(head->complen, "%d", compbytes);
      writeihdr(fp,head);
      n = fwrite(compdata,1,compbytes,fp);
      if (n != compbytes)
         syserr("writeihdrfile", "fwrite", file);
      free(compdata);
   }

   /* closes the new file */
   (void) fclose(fp);
}

/************************************************************/
/*         Routine:   Writeihdrsubimage()                   */
/*         Author:    Michael D. Garris                     */
/*         Date:      5/29/91                               */
/************************************************************/
/************************************************************/
/* Writeihdrsubimage() writes a subimage raster along with  */
/* a modified ihead structure to the specified file.        */
/************************************************************/
void writeihdrsubimage(
char *name,
unsigned char *data,
int w,int  h,int  d,
char *parent,
int par_x,int  par_y)
{
   IHEAD *ihead;

   if((ihead = (IHEAD *)malloc(sizeof(IHEAD))) == NULL)
      syserr("writeihdrsubimage", "malloc", "ihead");
   nullihdr(ihead);
   strcpy(ihead->id, name);
   strcpy(ihead->created,current_time());
   sprintf(ihead->width, "%d", w);
   sprintf(ihead->height, "%d", h);
   sprintf(ihead->depth, "%d", d);
   sprintf(ihead->compress, "%d", CCITT_G4);
   sprintf(ihead->align, "%d", 8);
   sprintf(ihead->unitsize, "%d", 8);
   ihead->sigbit = MSBF;
   ihead->byte_order = HILOW;
   sprintf(ihead->pix_offset, "%d", 0);
   sprintf(ihead->whitepix, "%d", 0);
   ihead->issigned = UNSIGNED;
   ihead->rm_cm = ROW_MAJ;
   ihead->tb_bt = TOP2BOT;
   ihead->lr_rl = LEFT2RIGHT;
   strcpy(ihead->parent, parent);
   sprintf(ihead->par_x, "%d", par_x);
   sprintf(ihead->par_y, "%d", par_y);
   writeihdrfile(ihead->id, ihead, data);
   free(ihead);
}

/************************************************************/
void write_fields( char *ofile,char  **name_list,
		int num_names, IHEAD **heads,
		unsigned char **fields, int count, int  compression)
{
  char outname[MAXPATHLEN], tail[MAXPATHLEN];
  int i;

  if(num_names != count)
     fatalerr("write_fields",
              "Table_A name list not equal to number of fields", NULL);
  for(i = 0; i < count; i++){
     sprintf(outname,"%s.%s", ofile, name_list[i]);
     strcpy(tail, outname);
     filetail(tail);
     set_id(heads[i], tail);
     set_compression(heads[i], compression);
     writeihdrfile(outname, heads[i], fields[i]);
   }
}

/*
#cat: write_ihdr_std - writes ihdr file with many "standard" fields
***/
/*******************************************************************/

/* Writes data to an ihdr file, putting "standard" values into many
ihdr fields */

void write_ihdr_std(unsigned char *data, const int width, const int height,
         const int depth, char *outfile)
{
  IHEAD head;
  char timestring[100];
  long thetime;

  nullihdr(&head);
  thetime = time(0);
  strcpy(timestring, ctime(&thetime));
  timestring[strlen(timestring) - 1] = 0;
  strcpy(head.created, timestring);
  sprintf(head.width, "%d", width);
  sprintf(head.height, "%d", height);
  sprintf(head.depth, "%d", depth);
  strcpy(head.compress, "0");
  strcpy(head.complen, "0");
  strcpy(head.align, "32");
  strcpy(head.unitsize, "32");
  head.sigbit = '0';
  head.byte_order = '0';
  strcpy(head.pix_offset, "0");
  strcpy(head.whitepix, "255");
  head.issigned = '0';
  head.rm_cm = '0';
  head.tb_bt = '0';
  head.lr_rl = '0';
  writeihdrfile(outfile, &head, data);
}
