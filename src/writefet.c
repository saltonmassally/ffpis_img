/***********************************************************************
      LIBRARY: FET - Feature File/List Utilities

      FILE:    WRITEFET.C
      AUTHOR:  Michael Garris
      DATE:    01/11/2001

      Contains routines responsible for writing the contents of
      an attribute-value paired list to a file.

      ROUTINES:
#cat: writefetfile - write the contents of an fet structure to the
#cat:                specified file.  Exits on error.
#cat: writefetfile_ret - write the contents of an fet structure to the
#cat:                specified file.  Returns on error.

***********************************************************************/

#include <stdio.h>
#include <fet.h>
#include <ffpis/util/util.h>

/*****************************************************************/
void writefetfile(char *file, FET *fet)
{
   FILE *fp = (FILE *)NULL;
   int item;

   if ((fp = fopen(file,"wb")) ==  (FILE *)NULL)
       syserr("writefetfile","fopen",file);
   for (item = 0; item<fet->num; item++){
       if(fet->values[item] == (char *)NULL)
          fprintf(fp,"%s\n",fet->names[item]);
       else
          fprintf(fp,"%s %s\n",fet->names[item],fet->values[item]);
   }
   fclose(fp);
}

/*****************************************************************/
int writefetfile_ret(char *file, FET *fet)
{
   FILE *fp = (FILE *)NULL;
   int item;

   if ((fp = fopen(file,"wb")) ==  (FILE *)NULL){
      fprintf(stderr, "ERROR : writefetfile_ret : fopen : %s\n",file);
      return(-2);
   }

   for (item = 0; item<fet->num; item++){
       if(fet->values[item] == (char *)NULL)
          fprintf(fp,"%s\n",fet->names[item]);
       else
          fprintf(fp,"%s %s\n",fet->names[item],fet->values[item]);
   }
   fclose(fp);

   return(0);
}
