/***********************************************************************
      PACKAGE: NIST Image Display

      FILE:    DPYNORM.C

      AUTHORS: Stan Janet
               Michael D. Garris
      DATE:    12/03/1990

      ROUTINES:
               dpynorm()

***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <values.h>
#include <sys/types.h>
#include <dpyimage.h>

/*********************************************************************/
int dpynorm(int argc, char *argv[])
{
   int ret;
   int done=0, align, bpi, bytes;
   u_int iw, ih, depth, whitepix;
   u_char *data;
   extern int optind, verbose;

   while ( !done && (optind < argc)) {
      if(ret = readfile(argv[optind],&data,&bpi,&iw,&ih,&depth,
                        &whitepix,&align))
	return(ret);

      if (depth == 1)
         bytes = howmany(iw,BITSPERBYTE) * ih;
      else if(depth == 8)
         bytes = iw * ih;
      else /* if(depth == 24) */
         bytes = iw * ih * 3;

      if (verbose > 2) {
         u_long zero, one;
         (void) printf("%s:\n",argv[optind]);
         (void) printf("\timage size: %u x %u (%d bytes)\n",
                       iw,ih,bytes);
         (void) printf("\tdepth: %u\n",depth);
                       pixelcount(data,(u_long)bytes,&zero,&one);
         (void) printf("\tpixel breakdown: %ld zero, %ld one\n\n",
                       zero,one);
      }

      if(ret = dpyimage(argv[optind],data,iw,ih,depth,whitepix,align,&done)){
         free((char *)data);
         return(ret);
      }

      free((char *)data);
      optind++;
   }

   return(0);
}

