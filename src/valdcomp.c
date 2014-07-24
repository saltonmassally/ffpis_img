/***********************************************************************
      LIBRARY: IHEAD - IHead Image Utilities

      FILE:    VALDCOMP.C
      AUTHOR:  Michael Garris
      DATE:    04/26/1989

      Contains routines responsible for determining value image
      compression algorithms for use in an IHead image.
      
      ROUTINES:
#cat: valid_compression - determines if the compression code passed is
#cat:                     supported by installed software.

***********************************************************************/

#include <ihead.h>
#include<defs.h>

int valid_compression( int code)
{
  switch (code){
    case UNCOMP:
         return(True);
         break;
    case CCITT_G3:
         return(True);
         break;
    case CCITT_G4:
         return(True);
         break;
    case RL:
         return(True);
         break;
    default:
         return(False);
         break;
  }
}
