/***********************************************************************
      LIBRARY: FET - Feature File/List Utilities

      FILE:    PRINTFET.C
      AUTHOR:  Michael Garris
      DATE:    01/11/2001

      Contains routines responsible for printing the contents of
      a data structure holding an attribute-value paired list.

      ROUTINES:
#cat: printfet - dumps the contents of an fet structure to the specified
#cat:            open file pointer.

***********************************************************************/

#include <stdio.h>
#include <fet.h>

void printfet(FILE *fp, FET *fet)
{
  int item;

  for (item = 0; item<fet->num; item++){
       if(fet->values[item] == (char *)NULL)
          fprintf(fp,"%s\n",fet->names[item]);
       else
          fprintf(fp,"%s %s\n",fet->names[item],fet->values[item]);
  }
}
