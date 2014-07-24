/***********************************************************************
      LIBRARY: FET - Feature File/List Utilities

      FILE:    READFET.C
      AUTHOR:  Michael Garris
      DATE:    01/11/2001

      Contains routines responsible for reading the contents of
      a file into a data structure holding an attribute-value
      paired list.

      ROUTINES:
#cat: readfetfile - opens an fet file and reads its contents into an
#cat:               fet structure.  Exits on error.
#cat: readfetfile_ret - opens an fet file and reads its contents into an
#cat:               fet structure.  Returns on error.

***********************************************************************/

#include <stdio.h>
#include <string.h>
#include <fet.h>
#include <ffpis/util/util.h>

/*****************************************************************/
FET *readfetfile(char *file)
{
   FILE *fp;
   FET *fet;
   char c,buf[MAXFETLENGTH];

   if ((fp = fopen(file,"rb")) == (FILE *)NULL)
      syserr("readfetfile","fopen",file);

   fet = allocfet(MAXFETS);
   while (fscanf(fp,"%s",buf) != EOF){
      while(((c = getc(fp)) == ' ') || (c == '\t'));
      ungetc(c, fp);
      if (fet->num >= fet->alloc)
         reallocfet(fet, fet->alloc + MAXFETS);
      fet->names[fet->num] = (char *)strdup(buf);
      if(fet->names[fet->num] == (char *)NULL)
         syserr("readfetfile","strdup","fet->names[]");
      fgets(buf,MAXFETLENGTH-1,fp);
      buf[strlen(buf)-1] = '\0';
      fet->values[fet->num] = (char *)strdup(buf);
      if(fet->values[fet->num] == (char *)NULL)
         syserr("readfetfile","strdup","fet->values[]");
      (fet->num)++;
   }
   fclose(fp);
   return(fet);
}

/*****************************************************************/
int readfetfile_ret(FET **ofet, char *file)
{
   int ret;
   FILE *fp;
   FET *fet;
   char c,buf[MAXFETLENGTH];

   if ((fp = fopen(file,"rb")) == (FILE *)NULL){
      fprintf(stderr, "ERROR : readfetfile_ret : fopen : %s\n", file);
      return(-2);
   }

   ret = allocfet_ret(&fet, MAXFETS);
   if(ret){
      fclose(fp);
      return(ret);
   }

   while (fscanf(fp,"%s",buf) != EOF){
      while(((c = getc(fp)) == ' ') || (c == '\t'));
      ungetc(c, fp);
      if (fet->num >= fet->alloc){
	 ret = reallocfet_ret(&fet, fet->alloc + MAXFETS);
         if(ret){
            fclose(fp);
            freefet(fet);
            return(ret);
         }
      }
      fet->names[fet->num] = (char *)strdup(buf);
      if(fet->names[fet->num] == (char *)NULL){
         fprintf(stderr, "ERROR : readfetfile_ret : strdup : fet->names[]\n");
         fclose(fp);
         freefet(fet);
         return(-3);
      }
      fgets(buf,MAXFETLENGTH-1,fp);
      buf[strlen(buf)-1] = '\0';
      fet->values[fet->num] = (char *)strdup(buf);
      if(fet->values[fet->num] == (char *)NULL){
         fprintf(stderr, "ERROR : readfetfile_ret : strdup : fet->values[]\n");
         fclose(fp);
         freefet(fet);
         return(-4);
      }
      (fet->num)++;
   }
   fclose(fp);
   *ofet = fet;

   return(0);
}
