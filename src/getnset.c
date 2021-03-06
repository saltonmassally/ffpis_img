/***********************************************************************
      LIBRARY: IHEAD - IHead Image Utilities

      FILE:    GETNSET.C
      AUTHOR:  Michael Garris
      DATE:    04/26/1989

      Contains routines responsible for retrieving and assigning
      attribute values to an IHead header.

      ROUTINES:
#cat: get_id - get the identity attribute from an IHead structure.
#cat:
#cat: set_id - set the identity attribute from an IHead structure.
#cat:
#cat: get_created - get the creation date attribute from an IHead structure.
#cat:
#cat: set_created - set the creation date attribute of an IHead structure.
#cat:
#cat: get_width - get the pixel width attribute from an IHead structure.
#cat:
#cat: set_width - set the pixel width attribute of an IHead structure.
#cat:
#cat: get_height - get the pixel height attribute from an IHead structure.
#cat:
#cat: set_height - set the pixel height attribute of an IHead structure.
#cat:
#cat: get_depth - get the pixel depth attribute from an IHead structure.
#cat:
#cat: set_depth - set the pixel depth attribute of an IHead structure.
#cat:
#cat: get_density - get the pixel density attribute from an IHead structure.
#cat:
#cat: set_density - set the pixel density attribute of an IHead structure.
#cat:
#cat: get_compression - get the compression code attribute from an
#cat:                   IHead structure.
#cat: set_compression - set the compression code  attribute of an
#cat:                   IHead structure.
#cat: get_complen - get the compressed data length attribute from
#cat:               an IHead structure.
#cat: set_complen - set the compressed data length attribute of
#cat:               an IHead structure.
#cat: get_align - get the scanline alignment attribute from an
#cat:             IHead structure.
#cat: set_align - set the scanline alignment attribute of an IHead structure.
#cat:
#cat: get_unitsize - get the memory unit size attribute from
#cat:                an IHead structure.
#cat: set_unitsize - set the memory unit size attribute of
#cat:                an IHead structure.
#cat: get_sigbit - get the bit order attribute from an IHead structure.
#cat:
#cat: set_sigbit - set the bit order attribute of an IHead structure.
#cat:
#cat: get_byte_order - get the byte order attribute from an IHead structure.
#cat:
#cat: set_byte_order - set the byte order attribute of an IHead structure.
#cat:
*/
/*
#cat: get_pix_offset - get the column pixel offset attribute from
#cat:                  an IHead structure.
#cat: set_pix_offset - set the column pixel offset attribute of
#cat:                  an IHead structure.
#cat: get_whitepix - get the white pixel attribute from an IHead structure.
#cat:
#cat: set_whitepix - set the white pixel attribute of an IHead structure.
#cat:
#cat: get_issigned - get the memory unit signed/unsigned attribute from
#cat:                an IHead structure.
#cat: set_issigned - set the memory unit signed/unsigned attribute of
#cat:                an IHead structure.
#cat: get_rm_cm - get the row/column major attribute from an IHead structure.
#cat:
#cat: set_rm_cm - set the row/column major attribute of an IHead structure.
#cat:
#cat: get_tb_bt - get the top-to-bottom/bottom-to-top attribute from
#cat:             an IHead structure.
#cat: set_tb_bt - set the top-to-bottom/bottom-to-top attribute of
#cat:             an IHead structure.
#cat: get_lr_rl - get the right-to-left/left-to-right attribute from
#cat:             an IHead structure.
#cat: set_lr_rl - set the right-to-left/left-to-right attribute of
#cat:             an IHead structure.
#cat: get_parent - get the parent image attribute from an IHead structure.
#cat:
#cat: set_parent - set the parent image attribute of an IHead structure.
#cat:
#cat: get_par_x - get the x-coordinate cut from parent attribute from
#cat:             an IHead structure.
#cat: set_par_x - set the x-coordinate cut from parent attribute of
#cat:             an IHead structure.
#cat: get_par_y - get the y-coordinate cut from parent attribute from
#cat:             an IHead structure.
#cat: set_par_y - set the y-coordinate cut from parent attribute of
#cat:             an IHead structure.

***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ihead.h>
#include <defs.h>
#include "getnset.h"
#include <ffpis/util/util.h>

char *get_id( IHEAD *head)
{
  if (head==NULL)
     fatalerr("get_id","pointer head is NULL",NULL);
  return((char *)strdup(head->id));
}

void set_id( IHEAD *i, char *path)
{
if (i == (IHEAD *) NULL)
	fatalerr("set_id","null ihead pointer",(char *)NULL);
if (path == (char *) NULL)
	fatalerr("set_id","null filename pointer",(char *)NULL);

while ((path != (char *) NULL) && (strlen(path) >= sizeof((*i).id)))
	path = strchr(path+1,'/');

memset(i->id,0,sizeof((*i).id));
if (path != (char *) NULL)
	(void) strcpy(i->id,path);
}

char *get_created( IHEAD *head)
{
  if (head==NULL)
     fatalerr("get_created","pointer head is NULL",NULL);
  return((char *)strdup(head->created));
}

void set_created( IHEAD *head)
{
  char *cur_time;

  if (head==NULL)
     fatalerr("set_created","pointer head is NULL",NULL);
  if ((cur_time = current_time()) == NULL)
     memset(head->created,0,DATELEN);
  else
    strcpy(head->created, cur_time);
}

int get_width( IHEAD *head)
{
  int width;

  if (head==NULL)
     fatalerr("get_width","pointer head is NULL",NULL);
  sscanf(head->width,"%d", &width);
  return(width);
}

int set_width( IHEAD *head, int width)
{
  if (head==NULL)
     fatalerr("set_width","pointer head is NULL",NULL);
  memset(head->width,0,SHORT_CHARS);
  sprintf(head->width,"%d",width);
  return(True);
}

int get_height( IHEAD *head)
{
  int height;

  if (head==NULL)
     fatalerr("get_height","pointer head is NULL",NULL);
  sscanf(head->height,"%d", &height);
  return(height);
}

int set_height( IHEAD *head, int height)
{
  if (head==NULL)
     fatalerr("set_height","pointer head is NULL",NULL);
  memset(head->height,0,SHORT_CHARS);
  sprintf(head->height,"%d",height);
  return(True);
}

int get_depth( IHEAD *head)
{
  int depth;

  if (head==NULL)
     fatalerr("get_depth","pointer head is NULL",NULL);
  sscanf(head->depth,"%d", &depth);
  return(depth);
}

int set_depth( IHEAD *head, int depth)
{
  if (head==NULL)
     fatalerr("set_depth","pointer head is NULL",NULL);
  memset(head->depth,0,SHORT_CHARS);
  sprintf(head->depth,"%d",depth);
  return(True);
}

int get_density( IHEAD *head)
{
  int density;

  if (head==NULL)
     fatalerr("get_density","pointer head is NULL",NULL);
  sscanf(head->density,"%d", &density);
  return(density);
}

int set_density( IHEAD *head, int density)
{
  if (head==NULL)
     fatalerr("set_density","pointer head is NULL",NULL);
  memset(head->density,0,SHORT_CHARS);
  sprintf(head->density,"%d",density);
  return(True);
}

int get_compression( IHEAD *head)
{
  int compression_code;

  if (head==NULL)
     fatalerr("get_compression","pointer head is NULL",NULL);
  sscanf(head->compress,"%d", &compression_code);
  return(compression_code);
}

int set_compression( IHEAD *head, int compression_code)
{
  if (head==NULL)
     fatalerr("set_compression","pointer head is NULL",NULL);
  if (valid_compression(compression_code)){
     memset(head->compress,0,SHORT_CHARS);
     sprintf(head->compress,"%d",compression_code);
  }
  else
     return(False);
  return(True);
}

int get_complen( IHEAD *head)
{
  int complen;

  if (head==NULL)
     fatalerr("get_complen","pointer head is NULL",NULL);
  sscanf(head->complen,"%d", &complen);
  return(complen);
}

int set_complen( IHEAD *head, int complen)
{
  if (head==NULL)
     fatalerr("set_complen","pointer head is NULL",NULL);
  memset(head->complen, 0, SHORT_CHARS);
  sprintf(head->complen,"%d",complen);
  return(True);
}

int get_align( IHEAD *head)
{
  int align;

  if (head==NULL)
     fatalerr("get_align","pointer head is NULL",NULL);
  sscanf(head->align,"%d", &align);
  return(align);
}

int set_align( IHEAD *head, int align)
{
  if (head==NULL)
     fatalerr("set_align","pointer head is NULL",NULL);
  memset(head->align,0,SHORT_CHARS);
  sprintf(head->align,"%d",align);
  return(True);
}

int get_unitsize( IHEAD *head)
{
  int unitsize;

  if (head==NULL)
     fatalerr("get_unitsize","pointer head is NULL",NULL);
  sscanf(head->unitsize,"%d", &unitsize);
  return(unitsize);
}

int set_unitsize( IHEAD *head, int unitsize)
{
  if (head==NULL)
     fatalerr("set_unitsize","pointer head is NULL",NULL);
  memset(head->unitsize,0,SHORT_CHARS);
  sprintf(head->unitsize,"%d",unitsize);
  return(True);
}

int get_sigbit( IHEAD *head)
{
  int sigbit;

  if (head==NULL)
     fatalerr("get_sigbit","pointer head is NULL",NULL);
  sigbit = atoi(&(head->sigbit));
  return(sigbit);
}

int set_sigbit( IHEAD *head, int sigbit)
{
  if (head==NULL)
     fatalerr("set_sigbit","pointer head is NULL",NULL);
  
  if((sigbit == 0) || (sigbit == '0'))
     head->sigbit = '0';
  else if((sigbit == 1) || (sigbit == '1'))
     head->sigbit = '1';
  else
     fatalerr("set_sigbit","Sigbit must equal 1 (MSB Last) or 0 (MSB First).",NULL);

  return(True);
}

int get_byte_order( IHEAD *head)
{
  int byte_order;

  if (head==NULL)
     fatalerr("get_byte_order","pointer head is NULL",NULL);
  byte_order = atoi(&(head->byte_order));
  return(byte_order);
}

int set_byte_order( IHEAD *head, int byte_order)
{
  if (head==NULL)
     fatalerr("set_byte_order","pointer head is NULL",NULL);
  head->byte_order = (char)byte_order;

  if((byte_order == 0) || (byte_order == '0'))
     head->byte_order = '0';
  else if((byte_order == 1) || (byte_order == '1'))
     head->byte_order = '1';
  else
     fatalerr("set_byte_order","Byte_order must equal 1 (Low-High) or 0 (High-Low).",NULL);

  return(True);
}

int get_pix_offset( IHEAD *head)
{
  int pix_offset;

  if (head==NULL)
     fatalerr("get_pix_offset","pointer head is NULL",NULL);
  sscanf(head->pix_offset,"%d", &pix_offset);
  return(pix_offset);
}

int set_pix_offset( IHEAD *head, int pix_offset)
{
  if (head==NULL)
     fatalerr("set_pix_offset","pointer head is NULL",NULL);
  memset(head->pix_offset,0,SHORT_CHARS);
  sprintf(head->pix_offset,"%d",pix_offset);
  return(True);
}

int get_whitepix( IHEAD *head)
{
  int whitepix;

  if (head==NULL)
     fatalerr("get_whitepix","pointer head is NULL",NULL);
  sscanf(head->whitepix,"%d", &whitepix);
  return(whitepix);
}

int set_whitepix( IHEAD *head, int whitepix)
{
  if (head==NULL)
     fatalerr("set_whitepix","pointer head is NULL",NULL);
  memset(head->whitepix,0,SHORT_CHARS);
  sprintf(head->whitepix,"%d",whitepix);
  return(True);
}

int get_issigned( IHEAD *head)
{
  int issigned;

  if (head==NULL)
     fatalerr("get_issigned","pointer head is NULL",NULL);
  issigned = atoi(&(head->issigned));
  return(issigned);
}

int set_issigned( IHEAD *head, int issigned)
{
  if (head==NULL)
     fatalerr("set_issigned","pointer head is NULL",NULL);
  
  if((issigned == 0) || (issigned == '0'))
     head->issigned = '0';
  else if((issigned == 1) || (issigned == '1'))
     head->issigned = '1';
  else
     fatalerr("set_issigned","Issigned must equal 1 (signed) or 0 (unsigned).",NULL);

  return(True);
}

int get_rm_cm( IHEAD *head)
{
  int rm_cm;

  if (head==NULL)
     fatalerr("get_rm_cm","pointer head is NULL",NULL);
  rm_cm = atoi(&(head->rm_cm));
  return(rm_cm);
}

int set_rm_cm( IHEAD *head, int rm_cm)
{
  if (head==NULL)
     fatalerr("set_rm_cm","pointer head is NULL",NULL);
  
  if((rm_cm == 0) || (rm_cm == '0'))
     head->rm_cm = '0';
  else if((rm_cm == 1) || (rm_cm == '1'))
     head->rm_cm = '1';
  else
     fatalerr("set_rm_cm","Rm_cm must equal 1 (Column Major) or 0 (Row Major).",NULL);

  return(True);
}

int get_tb_bt( IHEAD *head)
{
  int tb_bt;

  if (head==NULL)
     fatalerr("get_tb_bt","pointer head is NULL",NULL);
  tb_bt = atoi(&(head->tb_bt));
  return(tb_bt);
}

int set_tb_bt( IHEAD *head, int tb_bt)
{
  if (head==NULL)
     fatalerr("set_tb_bt","pointer head is NULL",NULL);
  
  if((tb_bt == 0) || (tb_bt == '0'))
     head->tb_bt = '0';
  else if((tb_bt == 1) || (tb_bt == '1'))
     head->tb_bt = '1';
  else
     fatalerr("set_tb_bt","Tb_bt must equal 1 (bottom2top) or 0 (top2bottom).",NULL);

  return(True);
}

int get_lr_rl( IHEAD *head)
{
  int lr_rl;

  if (head==NULL)
     fatalerr("get_lr_rl","pointer head is NULL",NULL);
  lr_rl = atoi(&(head->lr_rl));
  return(lr_rl);
}

int set_lr_rl( IHEAD *head, int lr_rl)
{
  if (head==NULL)
     fatalerr("set_lr_rl","pointer head is NULL",NULL);
  
  if((lr_rl == 0) || (lr_rl == '0'))
     head->lr_rl = '0';
  else if((lr_rl == 1) || (lr_rl == '1'))
     head->lr_rl = '1';
  else
     fatalerr("set_lr_rl","Lr_rl must equal 1 (right2left) or 0 (left2right).",NULL);

  return(True);
}

char *get_parent( IHEAD *head)
{
  if (head==NULL)
     fatalerr("get_parent","pointer head is NULL",NULL);
  return((char *)strdup(head->parent));
}

/* LINTLIBRARY */
/*
 * Stan Janet
 * 12/14/90
 *
 * Clears all bytes of parent field of iheader,
 * then sets the field to the longest subpath
 * that will fit in the space provided in the structure,
 * if any.
 */

void set_parent( IHEAD *i, char *path)
{
   if (i == (IHEAD *) NULL)
	fatalerr("set_parent","null ihead pointer",(char *)NULL);
   if (path == (char *) NULL)
	fatalerr("set_parent","null filename pointer",(char *)NULL);

   while ((path != (char *) NULL) && (strlen(path) >= sizeof((*i).parent)))
	path = strchr(path+1,'/');

   memset(i->parent,0,sizeof((*i).parent));
   if (path != (char *) NULL)
	(void) strcpy(i->parent,path);
}

int get_par_x( IHEAD *head)
{
  int par_x;

  if (head==NULL)
     fatalerr("get_par_x","pointer head is NULL",NULL);
  sscanf(head->par_x,"%d", &par_x);
  return(par_x);
}

int set_par_x( IHEAD *head, int par_x)
{
  if (head==NULL)
     fatalerr("set_par_x","pointer head is NULL",NULL);
  memset(head->par_x,0,SHORT_CHARS);
  sprintf(head->par_x,"%d",par_x);
  return(True);
}

int get_par_y( IHEAD *head)
{
  int par_y;

  if (head==NULL)
     fatalerr("get_par_y","pointer head is NULL",NULL);
  sscanf(head->par_y,"%d", &par_y);
  return(par_y);
}

int set_par_y( IHEAD *head, int par_y)
{
  if (head==NULL)
     fatalerr("set_par_y","pointer head is NULL",NULL);
  memset(head->par_y,0,SHORT_CHARS);
  sprintf(head->par_y,"%d",par_y);
  return(True);
}
