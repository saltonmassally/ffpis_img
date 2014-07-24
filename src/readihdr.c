/***********************************************************************
      LIBRARY: IHEAD - IHead Image Utilities

      FILE:    READIHDR.C
      AUTHOR:  Michael Garris
      DATE:    04/26/1989

      Contains routines responsible for reading an IHead header
      from an open file.

      ROUTINES:
#cat: readihdr - reads the contents of an open file pointer into an
#cat:            IHead structure.

***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ihead.h"
#include "grp4deco.h"
#include "invbyte.h"
#include "getnset.h"
#include "memalloc.h"
#include <ffpis/util/util.h>
#include "imgutil.h"
#include "imgdecod.h"
#include <math.h>

/************************************************************/
/* Readihdr() allocates and reads header information into an*/
/* ihead structure and returns the initialized structure.   */
/*                                                          */
/*         Modifications:                                   */
/*		1/11/91 Stan Janet                          */
/*			check return codes                  */
/*			declare malloc()                    */
/************************************************************/
IHEAD *readihdr( FILE *fp)
{
   IHEAD *head;
   char lenstr[SHORT_CHARS];
   int n, len;

   n = fread(lenstr,1,SHORT_CHARS,fp);
   if (n != SHORT_CHARS) {
	(void) fprintf(stderr,"readihdr: fread returned %d (expected %d)\n",
		n,SHORT_CHARS);
	exit(1);
   }

   if (sscanf(lenstr,"%d",&len) != 1)
	fatalerr("readihdr","cannot parse length field",(char *)NULL);

   if (len != IHDR_SIZE)
      fatalerr("readihdr","Record Sync Error: Header not found or old format.",
            NULL);

   head = (IHEAD *) malloc(sizeof(IHEAD));
   if (head == (IHEAD *) NULL)
      syserr("readihdr","malloc","head");

   n = fread(head,1,len,fp);
   if (n != len) {
	(void) fprintf(stderr,"readihdr: fread returned %d (expected %d)\n",
		n,len);
	exit(1);
   }

   return head;
}
/***********************************************************************
      LIBRARY: IMAGE - Image Manipulation and Processing Routines

      FILE:    READIHDR.C

      AUTHOR:  Michael Garris
      DATE:    04/28/1989

      Contains routines responsible for reading/reconstructing the
      pixmap contained in an IHead image file into memory.

      ROUTINES:
#cat: ReadBinaryRaster - reads the contents of a binary IHead image file
#cat:                   into an IHead structure and image memory.
#cat: ReadIheadRaster - reads the contents of a multi-level IHead image
#cat:                   file into an IHead structure and image memory.

***********************************************************************/

/************************************************************/
/*         Routine:   ReadBinaryRaster()                    */
/*         Author:    Michael D. Garris                     */
/*         Date:      4/28/89                               */
/*         Modifications:                                   */
/*           8/90 Stan Janet                                */
/*                     only malloc 1 buffer if data is not  */
/*                        compressed                        */
/*                     free() up temp buffer                */
/*           9/20/90 Stan Janet                             */
/*                     check return codes                   */
/*           1/11/91 Stan Janet                             */
/*                     put filename in error messages       */
/*           11/15/95 Patrick Grother			    */
/*                     use malloc instead of calloc	    */
/************************************************************/
/************************************************************/
/* ReadBinaryRaster() reads in a "headered" binary raster   */
/* file and returns an ihead structure, raster data, and    */
/* integer file specs.                                      */
/************************************************************/

void ReadBinaryRaster(
char *file,
IHEAD **head,
unsigned char **data,
int *bpi,int *width,int *height)
{
   FILE *fp;
   IHEAD *ihead;
   int outbytes, depth, comp, filesize, complen, n;
   unsigned char *indata, *outdata;

   /* open the image file */
   fp = fopen(file,"rb");
   if (fp == NULL)
      syserr("ReadBinaryRaster",file,"fopen");

   /* read in the image header */
   (*head) = readihdr(fp);
   ihead = *head;

   depth = get_depth(ihead);
   if(depth != 1)
      fatalerr("ReadBinaryRaster",file,"not a binary file");
   (*width) = get_width(ihead);
   (*height) = get_height(ihead);
   (*bpi) = get_density(ihead);
   comp = get_compression(ihead);
   complen = get_complen(ihead);

   /* allocate a raster data buffer */
   filesize = SizeFromDepth(*width, *height, depth);
   malloc_uchar(&outdata, filesize, "ReadIheadRaster : outdata");

   /* read in the raster data */
   if(comp == UNCOMP) {   /* file is uncompressed */
      n = fread(outdata,1,filesize,fp);
      if (n != filesize) {
	 (void) fprintf(stderr,
		"ReadBinaryRaster: %s: fread returned %d (expected %d)\n",
		file,n,filesize);
         exit(1);
      } /* IF */
   } else {
      malloc_uchar(&indata, complen, "ReadBinaryRaster : indata");
      n = fread(indata,1,complen,fp); /* file compressed */
      if (n != complen) {
         (void) fprintf(stderr,
		"ReadBinaryRaster: %s: fread returned %d (expected %d)\n",
		file,n,complen);
      } /* IF */
   }

   switch (comp) {
      case RL:
	 rldecomp(indata,complen,outdata,&outbytes,filesize);
         set_compression(ihead, UNCOMP);
         set_complen(ihead, 0);
         free((char *)indata);
         break;
      case CCITT_G4:
         if((*head)->sigbit == LSBF) {
           inv_bytes(indata, complen);
           (*head)->sigbit = MSBF;
           (*head)->byte_order = HILOW;
         }
	 grp4decomp(indata,complen,*width,*height,outdata,&outbytes);
         set_compression(ihead, UNCOMP);
         set_complen(ihead, 0);
         free((char *)indata);
         break;
      case UNCOMP:
         break;
      default:
         fatalerr("ReadBinaryRaster",file,"Invalid compression code");
      break;
   }

   *data = outdata;
   /* close the image file */
   (void) fclose(fp);
}

/************************************************************/
/*         Routine:   ReadIheadRaster()                     */
/*         Author:    Michael D. Garris                     */
/*         Date:      4/28/89                               */
/*         Modifications:                                   */
/*           8/90    (Stan Janet) see ReadBinaryRaster      */
/*           9/20/90 (Stan Janet)  "       "                */
/************************************************************/
/************************************************************/
/* ReadIheadRaster() reads in a "iheadered" raster file and */
/* returns an ihead structure, raster data, and integer file*/
/* specs.                                                   */
/************************************************************/
void ReadIheadRaster(
char *file,
IHEAD **head,
unsigned char **data,
int *width,int *height,int *depth)
{
   FILE *fp;
   IHEAD *ihead;
   int outbytes, comp, filesize, complen, n;
   unsigned char *indata, *outdata;

   /* open the image file */
   fp = fopen(file,"rb");
   if (fp == NULL)
      syserr("ReadIheadRaster",file,"fopen failed");

   /* read in the image header */
   *head = readihdr(fp);
   ihead = *head;

   n = sscanf((*head)->compress,"%d",&comp);
   if (n != 1)
      fatalerr("ReadIheadRaster",file,"sscanf failed on compress field");

   /* convert string fields to integers */
   n = sscanf((*head)->depth,"%d",depth);
   if (n != 1)
      fatalerr("ReadIheadRaster",file,"sscanf failed on depth field");
   n = sscanf((*head)->width,"%d",width);
   if (n != 1)
      fatalerr("ReadIheadRaster",file,"sscanf failed on width field");
   n = sscanf((*head)->height,"%d",height);
   if (n != 1)
      fatalerr("ReadIheadRaster",file,"sscanf failed on height field");
   n = sscanf((*head)->complen,"%d",&complen);
   if (n != 1)
      fatalerr("ReadIheadRaster",file,"sscanf failed on complen field");

   /* allocate a raster data buffer */
   filesize = SizeFromDepth(*width,*height,*depth);
   malloc_uchar(&outdata, filesize, "ReadIheadRaster : outdata");

   /* read in the raster data */
   if(comp == UNCOMP) {   /* file is uncompressed */
      n = fread(outdata,1,filesize,fp);
      if (n != filesize) {
         (void) fprintf(stderr,
		"ReadIheadRaster: %s: fread returned %d (expected %d)\n",
		file,n,filesize);
         exit(1);
      } /* IF */
   } else {
      malloc_uchar(&indata, complen, "ReadIheadRaster : indata");
      n = fread(indata,1,complen,fp); /* file compressed */
      if (n != complen) {
         (void) fprintf(stderr,
		"ReadIheadRaster: %s: fread returned %d (expected %d)\n",
		file,n,complen);
         exit(1);
      } /* IF */
   }

   switch (comp) {
      case RL:
	rldecomp(indata,complen,outdata,&outbytes,filesize);
	memset((*head)->complen,0,SHORT_CHARS);
	memset((*head)->compress,0,SHORT_CHARS);
        (void) sprintf((*head)->complen,"%d",0);
        (void) sprintf((*head)->compress,"%d",UNCOMP);
        *data = outdata;
        free((char *)indata);
      break;
      case CCITT_G4:
        if((*head)->sigbit == LSBF) {
          inv_bytes(indata, complen);
          (*head)->sigbit = MSBF;
          (*head)->byte_order = HILOW;
        }
        grp4decomp(indata,complen,*width,*height,outdata,&outbytes);
	memset((*head)->complen,0,SHORT_CHARS);
	memset((*head)->compress,0,SHORT_CHARS);
        (void) sprintf((*head)->complen,"%d",0);
        (void) sprintf((*head)->compress,"%d",UNCOMP);
        *data = outdata;
        free((char *)indata);
      break;
      case UNCOMP:
        *data = outdata;
      break;
      default:
         fatalerr("ReadIheadRaster",file,"Invalid compression code");
      break;
   }
   /* close the image file */
   (void) fclose(fp);
}
