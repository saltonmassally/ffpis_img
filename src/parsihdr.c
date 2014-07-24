/***********************************************************************
      LIBRARY: IHEAD - IHead Image Utilities

      FILE:    PARSIHDR.C
      AUTHOR:  Michael Garris
      DATE:    11/30/1989

      Contains routines responsible for parsing information stored
      in an IHead header's ID field.

      ROUTINES:
#cat: parseihdrid - takes the identity attribute from an hsf box image
#cat:               and parses out the box index and the reference string.

***********************************************************************/

#include <stdio.h>
#include <ihead.h>

/*******************************************************/
/*      Routine:  ParseIhdrID()                        */
/*      Author:   Michael D. Garris                    */
/*      Date:     11/30/89                             */
/*******************************************************/
/* Parseihdrid() takes a ihead id string and parses out*/
/* the index number and reference string.              */
/*******************************************************/
void parseihdrid( char *id,char *indxstr,char *refstr)
{
   char *iptr;

   iptr = id;
   /* EDIT MDG 1/25/99
   while((*iptr != NULL) && (*iptr != '_'))
   */
   while((*iptr != '\0') && (*iptr != '_'))
      iptr++;
   /* EDIT MDG 1/25/99
   if(*iptr == NULL){
   */
   if(*iptr == '\0'){
      /* start search for ref from beginning */
      iptr = id;
   }
   else{
      ++iptr;
      /* EDIT MDG 1/25/99
      while((*iptr != NULL) && (*iptr != '.'))
      */
      while((*iptr != '\0') && (*iptr != '.'))
         *indxstr++ = *iptr++;
   }
   /* EDIT MDG 1/25/99
   *indxstr = NULL;
   */
   *indxstr = '\0';
   /* EDIT MDG 1/25/99
   while((*iptr != NULL) && (*iptr != '\"'))
   */
   while((*iptr != '\0') && (*iptr != '\"'))
      iptr++;
   /* EDIT MDG 1/25/99
   if(*iptr != NULL){
   */
   if(*iptr != '\0'){
      ++iptr;
      /* EDIT MDG 1/25/99
      while((*iptr != NULL) && (*iptr != '\"'))
      */
      while((*iptr != '\0') && (*iptr != '\"'))
         *refstr++ = *iptr++;
   }
   /* EDIT MDG 1/25/99
   *refstr = NULL;
   */
   *refstr = '\0';
}
