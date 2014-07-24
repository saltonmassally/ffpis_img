/***********************************************************************
      PACKAGE: NIST Image Display

      FILE:    TALLY.C

      AUTHOR:  Stan Janet
      DATE:    12/03/1990

      ROUTINES:
               bitcount()
               bytecount()
               pixelcount()

***********************************************************************/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <values.h>

#define BYTE_RANGE	(1<<BITSPERBYTE)

/**********************************************************************/
int bitcount(register u_int c)
{
   register int b;

   for (b=0; c; c >>= 1){
      if (c & 01)
         b++;
   }

   return(b);
}

/**********************************************************************/
void bytecount(register u_char *data, register u_long n, register u_long *v)
{
   bzero((char *)v,BYTE_RANGE*sizeof(u_long));
   while (n-- > 0)
      v[*data++]++;
}

/**********************************************************************/
void pixelcount(register u_char *data, register u_long bytes,
                register u_long *zero, register u_long *one)
{
   u_int i;
   static u_char counts[BYTE_RANGE];
   static int counts_initialized = 0;

   if (! counts_initialized) {
      counts_initialized = 1;
      bzero((char *)counts,BYTE_RANGE);
      for (i=0; i < BYTE_RANGE; i++)
         counts[i] = bitcount(i);
   }

   *zero = 0L;
   *one = 0L;
   while (bytes-- > 0) {
      i = counts[*data++];
      *one += i;
      *zero += (BITSPERBYTE - i);
   }
}
