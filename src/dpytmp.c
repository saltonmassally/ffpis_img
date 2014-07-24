/***********************************************************************
      PACKAGE: NIST Image Display

      FILE:    DPYTMP.C

      AUTHORS: Stan Janet
               Michael D. Garris
      DATE:    12/03/1990

      ROUTINES:
               tmpcomm()
               tmp_parent()
               tmp_child()

***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <values.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dpyimage.h>

/*****************************************************************/
int tmpcomm(int argc, char **argv)
{
   int ret;
   int pid, p;
   char dir[MAXPATHLEN];

   if (verbose)
      (void) printf("In tmpcomm()\n");

   p = getpid();
   (void) sprintf(dir,OUTFILE_DIRFMT,tmpdir,p);
   if (mkdir(dir,OUTFILE_DIRMODE) < 0) {
      perror(dir);
      return(-2);
   }

   pid = fork();
   if (pid < 0) {
      perror("Fork failed");
      return(-3);
   }

   if (pid) {
      char cmd[2*MAXPATHLEN];

      if(ret = tmp_parent(p))
         return(ret);
      (void) kill(pid,SIGKILL);
      (void) sprintf(cmd,"/bin/rm -f %s/*",dir);
      if (verbose)
         (void) printf("%s\n",cmd);
         (void) system(cmd);
      if (rmdir(dir) < 0)
         perror(dir);
   } else {
      extern int nicevalue;
      extern char *program;

      program = "[child]";
      if ((nicevalue >= 0) && (nice(nicevalue) < 0))
         perror("Nice failed");
      if (ret = fdclose(0,"stdin"))
         return(ret);
      if (open("/dev/null",O_RDWR) < 0) {
         perror("Cannot open /dev/null");
         return(-4);
      }
      if(ret = fdclose(ConnectionNumber(display),"X11 connection"))
         return(ret);
      if(ret = tmp_child(argc,argv,p))
         return(ret);
      return(0);
   }

   return(0);
}

/************************************************************************/
int tmp_parent(int pid)
{
   int ret;
   register FILE *fp;
   int done=False, bytes, filenumber=0;
   u_char *data;
   u_int iw, ih, depth;
   char outfile[MAXPATHLEN], ctrlfile[MAXPATHLEN];
   struct header_t header;
   struct stat s;
   extern int errno;
   extern char *malloc();

   for (;;) {
      (void) sprintf(outfile,OUTFILE_FMT,tmpdir,pid,filenumber++);
      (void) strcpy(ctrlfile,outfile);
      (void) strcat(ctrlfile,OUTFILE_EXT);

      /* wait for creation of control file */
      while (access(ctrlfile,F_OK) < 0)
         sleep((u_int)1);

      if (verbose)
         (void) printf("\tcontrol file %s exists\n",ctrlfile);

      fp = fopen(outfile,"rb");
      if (fp == (FILE *) NULL) {
         (void) fprintf(stderr,"dpyimage: cannot open %s\n",outfile);
         return(-2);
      }

      unlinkfile(outfile);
      unlinkfile(ctrlfile);

      if (fstat(fileno(fp),&s) < 0) {
         perror(outfile);
         return(-3);
      }
      if (s.st_size == 0)
         break;

      if(ret = readheader(fp,&header))
         return(ret);

      iw = header.iw;
      ih = header.ih;
      depth = header.depth;
      if (depth == 1)
         bytes = howmany(iw,BITSPERBYTE) * ih;
      else if (depth == 8)
         bytes = iw * ih;
      else /* if(depth == 24) */
         bytes = iw * ih * 3;

      data = (u_char *) malloc((u_int) bytes);
      if (data == (u_char *) NULL) {
         (void) fprintf(stderr,"dpyimage: malloc(%d) failed\n", bytes);
         return(-4);
      }

      if (verbose > 2) {
         u_long zero, one;

         (void) printf("%s:\n",header.filename);
         (void) printf("\timage size: %u x %u (%d bytes)\n", iw,ih,bytes);
         (void) printf("\tdepth: %u\n",depth);
         pixelcount(data,(u_long)bytes,&zero,&one);
         (void) printf("\tpixel breakdown: %ld zero, %ld one\n\n",
                       zero,one);
      }

      if(ret = readdata(fp,data,bytes)){
         free((char *)data);
         return(ret);
      }
      if(ret = fclose(fp)){
         free((char *)data);
         return(ret);
      }
      if(ret = dpyimage(header.filename,data,iw,ih,depth,
                        header.whitepix,header.align,&done)){
         free((char *)data);
         return(ret);
      }
      free((char *)data);

      if (done)
         break;
   } /* for */

   return(0);
}

/*************************************************************/
int tmp_child(int argc, char **argv, int ppid)
{
   int ret;
   char outfile[MAXPATHLEN], ctrlfile[MAXPATHLEN];
   register FILE *fp;
   int align, bpi, bytes, filenumber=0;
   u_char *data;
   u_int iw, ih, depth, whitepix;
   struct header_t header;
   extern int optind;

   while ( optind <= argc ) {

      (void) sprintf(outfile,OUTFILE_FMT,tmpdir,ppid,filenumber++);
      (void) strcpy(ctrlfile,outfile);
      (void) strcat(ctrlfile,OUTFILE_EXT);

      fp = fopen(outfile,"wb");
      if (fp == (FILE *) NULL) {
         (void) fprintf(stderr,"dpyimage: cannot open %s\n",outfile);
         return(-2);
      }

      if (optind != argc) {
         if(ret = readfile(argv[optind],&data,&bpi,&iw,&ih,&depth,
                           &whitepix,&align))
            return(ret);
         buildheader(&header,argv[optind],iw,ih,depth,whitepix,align);
         if(ret = writeheader(fp,&header)){
            free((char *) data);
            return(ret);
         }
         if (depth == 1)
            bytes = howmany(iw,BITSPERBYTE) * ih;
         else if (depth == 8)
            bytes = iw * ih;
         else /* if (depth == 24) */
            bytes = iw * ih * 3;

         if (verbose)
            (void) printf("(child) %d bytes\n",bytes);
         if(ret = writedata(fp,data,bytes)){
            free((char *) data);
            return(ret);
         }
      } /* If */

      if (optind != argc)
         free((char *) data);
      if(ret = fclose(fp))
         return(ret);
      if(ret = createfile(ctrlfile))
         return(ret);

      optind++;
   } /* for */

   return(0);
}
