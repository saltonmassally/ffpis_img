/***********************************************************************
      LIBRARY: IHEAD - IHead Image Utilities

      FILE:    GETCOMP.C
      AUTHOR:  Michael Garris
      DATE:    04/26/1989

      Contains routines responsible for managing compression codes
      and string names.

      ROUTINES:
#cat: getcomptype - given a compression string, returns the corresponding
#cat:               compression code defined in ihead.h .

***********************************************************************/

#include <stdio.h>
#include <strings.h>
#include <string.h>

#include <ihead.h>


/*
 * Stan Janet
 * 2/22/91
 *
 * Translate compression string specifier to integer as defined in
 *	ihead.h, or -1 if bad string
 */


int getcomptype( char *s)
{
if (strcmp(s,"g3") == 0)
	return CCITT_G3;

if (strcmp(s,"g4") == 0)
	return CCITT_G4;

if ((strcmp(s,"uncomp") == 0))
	return UNCOMP;

return -1;
}
