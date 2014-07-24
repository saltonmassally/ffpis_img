/***********************************************************************
      PACKAGE: NIST Image Display

      FILE:    DPYPIPE.C

      AUTHORS: Stan Janey
               Michael D. Garris
      DATE:    12/03/1990

      ROUTINES:
               pipecomm()
               pipe_parent()
               pipe_child()

***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <values.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dpyimage.h>

/****************************************************************/
int pipecomm(int argc, char **argv)
{
   int ret, display_fd;
   register FILE *fp;
   int pid, cmdfd[2], datafd[2];
   int status, sig;

   if (verbose)
      (void) printf("In pipecomm()\n");

   if ((pipe(cmdfd) < 0) || (pipe(datafd) < 0)) {
      perror("Pipe failed");
      return(-2);
   }

   display_fd = ConnectionNumber(display);

   pid = fork();
   if (pid < 0) {
      perror("Fork failed");
      return(-3);
   }

   if (pid) {
      /* parent */

      if(ret = fdclose(cmdfd[0],"pipe"))
         return(ret);
      if(ret = fdclose(datafd[1],"pipe"))
         return(ret);
      fp = (FILE *)fdopen(datafd[0],"r");
      if (fp == (FILE *) NULL) {
         (void) fprintf(stderr,"%s: fdopen failed\n",program);
         return(-4);
      }
      if(ret = pipe_parent(fp))
         return(ret);
        
      (void) fclose(fp);

      if(waitpid(pid, &status, 0) < 0){
         perror("Waitpid failed");
         return(-5);
      }

      if(WIFEXITED(status) == 0){
         if(WIFSIGNALED(status)){
            sig = WTERMSIG(status);
            fprintf(stderr, "Child pid = %d died with Signal %d\n",
                    pid, sig);
            return(-6);
         }
      }

/*
      (void) kill(pid,SIGKILL);
*/
   }
   else {
      /* child */

      extern int nicevalue;
      extern char *program;

      program = "[child]";
      if ((nicevalue >= 0) && (nice(nicevalue) < 0))
         perror("Nice failed");
      if (ret = fdclose(0,"stdin"))
         return(-4);
      if (open("/dev/null",O_RDWR) < 0) {
         perror("Cannot open /dev/null");
         return(-7);
      }
      if(ret = fdclose(display_fd,"X11 connection"))
         return(ret);
      if(ret = fdclose(cmdfd[1],"pipe"))
         return(ret);
      if(ret = fdclose(datafd[0],"pipe"))
         return(ret);
      fp = (FILE *)fdopen(datafd[1],"w");
      if (fp == (FILE *) NULL) {
         (void) fprintf(stderr,"%s: fdopen failed\n",program);
         return(-8);
      }
      if(ret = pipe_child(argc,argv,fp))
         return(ret);
      (void) fclose(fp);
      exit(0);
   }

   return(0);
}

/****************************************************************/
int pipe_parent(register FILE *fp)
{
   int ret;
   int done=False, n, bytes;
   u_int iw, ih, depth;
   u_char *data;
   struct header_t header;
   extern char *malloc();

   while (! done) {
      n = fread((char *)&header,1,HEADERSIZE,fp);
      if (n == 0)
         break;
      if (n != HEADERSIZE) {
         (void) fprintf(stderr,
                        "%s: header fread returned %d, expected %d\n",
                        program,n,HEADERSIZE);
         return(-2);
      }

      iw = header.iw;
      ih = header.ih;
      depth = header.depth;
      if (depth == 1)
         bytes = howmany(iw,BITSPERBYTE) * ih;
      else if (depth == 8)
         bytes = iw * ih;
      else /* if (depth == 24) */
         bytes = iw * ih * 3;

      if (verbose) {
         (void) printf("%s:\n",header.filename);
         (void) printf("\timage size: %u x %u (%d bytes)\n",
                       iw,ih,bytes);
         (void) printf("\tdepth: %u\n",depth);
      }

      data = (u_char *) malloc((u_int) bytes);
      if (data == (u_char *) NULL) {
         (void) fprintf(stderr,"%s: malloc(%d) failed\n",
                        program,bytes);
         return(-3);
      }

      if(ret = readdata(fp,data,bytes))
         return(ret);

      if (verbose > 2) {
         u_long zero, one;
         pixelcount(data,(u_long)bytes,&zero,&one);
         (void) printf("\tpixel breakdown: %ld zero, %ld one\n\n",
                       zero,one);
      }

      if(ret = dpyimage(header.filename,data,iw,ih,depth,
                        header.whitepix,header.align,&done))
         return(ret);

      free((char *) data);
   } /* While */

   return(0);
}

/*******************************************************************/
int pipe_child(int argc, char **argv, register FILE *fp)
{
   int ret;
   int done = False, align, bpi, bytes;
   u_int iw, ih, depth, whitepix;
   u_char *data;
   extern int optind;
   struct header_t header;

   while ( !done && (optind < argc)) {
      if(ret = readfile(argv[optind],&data,&bpi,&iw,&ih,
                        &depth,&whitepix,&align))
         return(ret);
      buildheader(&header,argv[optind],iw,ih,depth,whitepix,align);
      if(ret = writeheader(fp,&header))
         return(ret);
      if (depth == 1)
         bytes = howmany(iw,BITSPERBYTE) * ih;
      else if(depth == 8)
         bytes = iw * ih;
      else /* if(depth == 24) */
         bytes = iw * ih * 3;

      if (verbose)
         (void) printf("(child) %d bytes\n",bytes);
      if(ret = writedata(fp,data,bytes))
         return(ret);
      (void) fflush(fp);
      free((char *) data);
      optind++;
   } /* While */

   return(0);
}

