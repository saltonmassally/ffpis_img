/***********************************************************************
      PACKAGE: NIST Image Display

      FILE:    DPYX.H

      AUTHORS: Stan Janet
               Michael D. Garris
      DATE:    12/03/1990

***********************************************************************/
#ifndef _DPYX_H
#define _DPYX_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>

#include <values.h>
#include <display.h>
#include <dpydepth.h>
#include <event.h>
#include <dpy.h>

#define WIN_XY_INCR             25

#define BITMAP_UNIT_24          4 /* 4 bytes ==> 32 bits */

/* FIXME this define is gross FIXME */
//#define PT(x,y,w,h)             (((x)>=0)&&((x)<(w))&&((y)>=0)&&((y)<(h)))
static inline
int PT(int x,int y,unsigned int w, unsigned int h)
{
  return (x)>=0&& (x)<(w)&&
	  (y)>=0&& (y)<(h);
}

#define ALL_BUTTONS  ((u_int)  (Button1Mask| \
                                Button2Mask| \
                                Button3Mask| \
                                Button4Mask| \
                                Button5Mask))


/* X-Window global references. */
extern Display *display;
extern char *display_name;
extern Window window, rw;
extern Visual *visual;
extern int screen;
extern Colormap def_cmap, cmap;
extern int cmap_size;
extern GC gc, boxgc, pointgc;
extern u_long bp, wp;
extern u_int border_width;
extern int no_window_mgr;
extern int no_keyboard_input;

/************************************************************************/
/* dpyx.c */
extern void cleanup(void);
extern int xconnect(void);
extern int initwin(u_int wx, u_int wy, u_int ww, u_int wh, u_int depth,
                   u_long wp);
extern int set_gray_colormap(Display *, Colormap, u_int, u_long);
extern int gray_colormap(Colormap *, Display *, Visual **, u_int);

#endif  /* !_DPYX_H */
