/***********************************************************************
      PACKAGE: NIST Image Display

      FILE:    DPYMAIN.C

      AUTHORS: Stan Janet
               Michael D. Garris
      DATE:    12/03/1990

#cat: dpyimage - Takes an IHead, WSQ, JPEGB, JPEGL, or raw image file,
#cat:            reconstructs the image pixmap (if needed), and renders
#cat:            the pixmap in an X11 Window.

***********************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dpyimage.h>

void procargs(int, char **);
void usage();

/*********************************************************************/

int main(int argc, char *argv[])
{
   int ret;
   extern int optind, dpy_mode, verbose;

   setlinebuf(stdout);

   procargs(argc,argv);
   if (optind >= argc)
      usage();

   if(ret = xconnect())
      exit(ret);

   screen = DefaultScreen(display);
   def_cmap = DefaultColormap(display,screen);
   dw = DisplayWidth(display,screen);
   dh = DisplayHeight(display,screen);
   rw = RootWindow(display,screen);
   visual = DefaultVisual(display,screen);
   bp = BlackPixel(display,screen);
   wp = WhitePixel(display,screen);

   if ((optind == argc - 1) || (dpy_mode == DPY_NORM)){
      if(ret = dpynorm(argc,argv))
         exit(ret);
   }
   else if (dpy_mode == DPY_PIPE){
      if (ret = pipecomm(argc,argv))
         exit(ret);
   }
   else {                          /* dpy_mode == DPY_TMP */
      extern char *tmpdir, def_tmpdir[];

      if (tmpdir == def_tmpdir) {  /* if not set on command line,  */
         char *p;		   /* check environment for TMPDIR */
         extern char *getenv();

         p = getenv("TMPDIR");
         if (p != (char *) NULL)
            tmpdir = p;
      }
      tmpcomm(argc,argv);
   }

   cleanup();

   if (verbose)
      (void) printf("Errors: %d\n",errors);

   exit(errors?1:0);
}

/************************************************************************/
void procargs(int argc, char **argv)
{
   int c;
   char *option_spec = "Aa:b:D:d:H:kN:Onr:s:T:tvW:xX:Y:";
   extern int atoi(), getopt(), optind, debug;
   extern char *optarg, *display_name;

   program = (char *)rindex(*argv,'/');
   if (program == (char *) NULL)
      program = *argv;
   else
      program++;

   while ((c = getopt(argc,argv,option_spec)) != EOF){
      switch (c) {
         case 'A':	automatic = True;
              break;

         case 'a':	accelerator = MAX(1,atoi(optarg));
              break;

         case 'b':	border_width = atoi(optarg);
              break;

         case 'D':	tmpdir = optarg;
              break;

         case 'd':	display_name = optarg;
              break;

         case 'H':	init_wh = atoi(optarg);
              break;

         case 'N':	nicevalue = atoi(optarg);
              break;

         case 'n':	dpy_mode = DPY_NORM;
              break;

         case 'r':	raw = True;
              c = sscanf(optarg,"%u,%u,%u,%u",
                         &raw_w,&raw_h,
                         &raw_depth,&raw_whitepix);
              if (c != 4) {
                 (void) fprintf(stderr,
                                "%s: cannot parse raw parameters\n", program);
                 usage();
              }
              break;

         case 's':	sleeptime = atoi(optarg);
              break;

         case 'T':	title = optarg;
              break;

         case 't':	dpy_mode = DPY_TMP;
              break;

         case 'v':	verbose++;
              break;

         case 'X':	wx = atoi(optarg);
              break;

         case 'x':	debug++;
              break;

         case 'Y':	wy = atoi(optarg);
              break;

         case 'W':	init_ww = atoi(optarg);
              break;

         case 'O':	no_window_mgr = True;
              break;

         case 'k':	no_keyboard_input = True;
              break;

         default:	usage();
              break;
      }/* switch */
   }
}

/****************************************************************
Print usage message and exit.
****************************************************************/
void usage(void)
{
   static char usage_msg[] = "\
   Usage:\n\
   %s [options] image-file ...\n\
	-r w,h,d,wp	files are raw data with given width, height, depth\n\
				and white pixel\n\
	-A		auto advance through images\n\
	-s n		sleep n seconds before advancing [%d]\n\
	-a n		set drag accelerator [1]\n\
	-v		verbose\n\
	-x		debug mode (create core dump on X11 error)\n\
	-b n		set border width to n [%d]\n\
	-N n		nice I/O process with increment n\n\
	-O		override redirect on window (no window manager)\n\
	-k		no keyboard input\n\
	-W n		set window width to n\n\
	-H n		set window height to n\n\
	-X n		set window x pixels from display border [0]\n\
	-Y n		set window y pixels from display border [0]\n\
	-n		do not fork; one process reads and displays images\n\
	-T title	set title for images [filename]\n\
	-t		transfer images by temporary files\n\
	-D dir		create temporary files in directory [%s]\n\
	-d display	connect to alternate X11 server\n";

   (void) fprintf(stderr,
	          usage_msg,
                  program,DEF_SLEEPTIME,DEF_BORDER_WIDTH,DEF_TMPDIR);
   exit(1);
}
void print_usage(void)
{
  usage();
}
