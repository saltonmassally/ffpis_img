/***********************************************************************
      PACKAGE: NIST Image Display

      FILE:    DPYX.C

      AUTHORS: Stan Janet
               Michael D. Garris
      DATE:    12/03/1990

      ROUTINES:
               cleanup()
               xconnect()
               intiwin()
               set_gray_colormap()
               gray_colormap()

***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dpyx.h>

/* X-Window globals. */
Display *display;
char *display_name = (char *) NULL;
Window window, rw;
Visual *visual;
int screen;
Colormap def_cmap, cmap;
int cmap_size;
GC gc, boxgc, pointgc;
u_long bp, wp;
u_int border_width = DEF_BORDER_WIDTH;
int no_window_mgr = False;
int no_keyboard_input = False;

/******************************************************************/
void cleanup()
{
   XFreeGC(display,boxgc);
   XFreeGC(display,pointgc);

   XUnmapWindow(display,window);
   XDestroyWindow(display,window);

   if((visual->class != TrueColor) &&
      (cmap != def_cmap))
      XFreeColormap(display,cmap);

   XCloseDisplay(display);
}

/************************************************************************/
int xconnect()
{
   display = XOpenDisplay(display_name);
   if (display == (Display *) NULL) {
      fprintf(stderr, "ERROR : xconnect : ");
      fprintf(stderr, "cannot connect to X11 server %s\n",
              XDisplayName(display_name));
      return(-2);
   }

   /* Return normally. */
   return(0);
}

/*****************************************************************/
int initwin(u_int wx, u_int wy, u_int ww, u_int wh, u_int depth,
             u_long wp)
{
  int ret;

   if (depth == 1) {

      cmap = def_cmap;
      window = XCreateSimpleWindow(display,rw,wx,wy,ww,wh,border_width,bp,wp);

   } else if (depth == BITSPERBYTE) {
      /* if default visual is TrueColor, then no colormap to manipulate */
      if(visual->class == TrueColor){
         window = XCreateSimpleWindow(display,rw,wx,wy,ww,wh,
                                      border_width,bp,wp);
      }
      /* otherwise assume PsuedoColor or GrayScale Visual ... */
      else{

         u_long wmask;
         XSetWindowAttributes wattr;

         ret = gray_colormap(&cmap, display,&visual,depth);
         if (ret || (cmap == None)) {
            fprintf(stderr,"ERROR : initwin : cannot obtain gray colormap\n");
            return(-2);
         }

         cmap_size = 1 << depth;
         if(ret = set_gray_colormap(display,cmap,cmap_size,wp))
            return(ret);

         wattr.colormap = cmap;
         wmask = CWColormap;
         window = XCreateWindow(display,rw,wx,wy,ww,wh,border_width,
                                CopyFromParent,CopyFromParent,
                                visual,wmask,&wattr);
      }
   }
   else if(depth == 24){
      /* if default visual is TrueColor, then no colormap to manipulate */
      if(visual->class == TrueColor){
	   window = XCreateSimpleWindow(display,rw,wx,wy,ww,wh,
                                       border_width,bp,wp);
      }
      /* otherwise assume PsuedoColor, which is not supported ... */
      else{
	   fprintf(stderr, "PsuedoColor is not currently supported for RGB\n");
           return(-3);
      }
   }
   else {
      fprintf(stderr, "ERORR : initwin : ");
      fprintf(stderr, "depth (%u) must be either 1, %d or 24\n",
              depth, BITSPERBYTE);
      return(-4);
   }

   gc = XDefaultGC(display,screen);
   boxgc = XCreateGC(display,rw,(u_long)0L,(XGCValues *)NULL);
   XSetFunction(display,boxgc,GXinvert);
   pointgc = XCreateGC(display,rw,(u_long)0L,(XGCValues *)NULL);
   XSetForeground(display,pointgc,0xFF0000);
   XSetFunction(display,pointgc,GXcopy);

   {
      XSizeHints size_hints;
      size_hints.x = wx;
      size_hints.y = wy;
      size_hints.width = ww;
      size_hints.height = wh;
      size_hints.flags = (USSize | USPosition);
      XSetStandardProperties(display,window,"","",None,(char **)NULL,
                             0,&size_hints);
   }

   if (no_window_mgr) {
      XSetWindowAttributes winattr;

      winattr.override_redirect = True;
      XChangeWindowAttributes(display,window,CWOverrideRedirect,&winattr);
   }
   else {
      XWMHints wmhints;

      wmhints.flags = InputHint;
      wmhints.input = True;
      XSetWMHints(display,window,&wmhints);
   }

   {
      u_long inputmask;

      inputmask = ( ExposureMask | ButtonPressMask | ButtonReleaseMask );
      if (! no_keyboard_input)
         inputmask |= ( KeyPressMask | KeyReleaseMask );
         XSelectInput(display,window,inputmask);
   }

   XMapRaised(display,window);

   /* Return normally. */
   return(0);
}

/*****************************************************************/
/* Sets the colormap intensities to be gray (equal r,g&b values) */
/*	and evenly spread out between black (0) and white        */
/*	(the maximum value for a u_short).                       */
/*****************************************************************/
int set_gray_colormap(Display *display, Colormap cmap,
                      u_int cmap_size, u_long wp)
{
   register u_long i;
   u_short m;
   u_short flags = (DoRed | DoGreen | DoBlue);
   XColor *cmap_defs;
   extern char *malloc();

   if (! cmap_size){
      fprintf(stderr, "ERROR : set_gray_colormap : colormap size is zero\n");
      return(-2);
   }

   if (wp && (wp != (u_long)cmap_size - 1L)){
      fprintf(stderr, "ERROR : set_gray_colormap : ");
      fprintf(stderr, "white pixel must be zero or (colormap size - 1)\n");
      return(-3);
   }

   cmap_defs = (XColor *) malloc((u_int) (cmap_size * sizeof(XColor)));
   if (cmap_defs == (XColor *) NULL){
      fprintf(stderr, "ERROR : set_gray_colormap : malloc failed\n");
      return(-4);
   }

   for (i=0; i < cmap_size; i++) {
      cmap_defs[i].pixel = i;
      m = ((wp ? i : ((cmap_size-1)-i)) / (double) (cmap_size - 1)) *
           ((u_short) ~0) + 0.5;
#ifdef DEBUG
      (void) printf("m[%d] = %u\n",i,m);
#endif
      cmap_defs[i].red   = m;
      cmap_defs[i].green = m;
      cmap_defs[i].blue  = m;
      cmap_defs[i].flags = flags;
   }

   XStoreColors(display,cmap,cmap_defs,cmap_size);

   free((char *) cmap_defs);

   return(0);
}

/************************************************************************/
/* Returns a gray colormap of the specified depth, setting *bp and *wp  */
/* to the correct values for the blackest and whitest pixels.           */
/* Visual should be set to the DefaultVisual. If the class for this     */
/*	visual is non-static (GrayScale or PseudoColor), a suitable     */
/*	visual must be found. If it is, visual is reset; otherwise      */
/*	returns None.                                                   */
/* A fatalerr error occurs if XCreateColormap() fails.                  */
/************************************************************************/
int gray_colormap(Colormap *ocmap, Display *display,
                  Visual **visual, u_int depth)
{
   XVisualInfo vinfo;
   Colormap cmap;
   int screen;

   screen = DefaultScreen(display);

   if (((*visual)->class != GrayScale) && ((*visual)->class != PseudoColor)) {
      if ((! XMatchVisualInfo(display,screen,depth,GrayScale,&vinfo)) &&
         (! XMatchVisualInfo(display,screen,depth,PseudoColor,&vinfo))) {
#ifdef DEBUG
         (void) fprintf(stderr,"Cannot find a suitable visual\n");
#endif
         *ocmap = (Colormap) None;
      }
      *visual = vinfo.visual;
   }

   cmap = XCreateColormap(display,RootWindow(display,screen),*visual,AllocAll);
   if (cmap == None){
      fprintf(stderr, "ERROR : gray_colormap : XCreateColormap failed\n");
      return(-2);
   }

   *ocmap = cmap;

   return(0);
}
