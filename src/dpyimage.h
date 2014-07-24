/***********************************************************************
      PACKAGE: NIST Image Display

      FILE:    DPYIMAGE.H

      AUTHORS: Stan Janet
               Michael D. Garris
      DATE:    12/03/1990

***********************************************************************/
#ifndef _DPYIMAGE_H
#define _DPYIMAGE_H

#include <dpyx.h>

/* X-Window global references. */
extern u_int dw, dh;
extern int window_up;
extern int got_click;
extern u_int depth;
extern u_int ww, wh, iw, ih;
extern int absx, absy, relx, rely;
extern int x_1, y_1;

/* X-Window Contols & command line globals. */
extern char *program;
extern char *filename;
extern int accelerator;
extern u_int init_ww, init_wh;
extern int nicevalue;
extern int pointwidth;
extern char *title;
extern u_int wx, wy;
extern int verbose;
extern int debug;
extern int errors;

extern int automatic;
extern u_int sleeptime;
extern int dpy_mode;
extern int raw;
extern u_int raw_w, raw_h, raw_depth, raw_whitepix;
extern char def_tmpdir[];
extern char *tmpdir;

extern int nist_flag;
extern int iafis_flag;

/************************************************************************/
/* dpyimage.c */
extern int dpyimage(char *, register u_char *, u_int, u_int,
                    u_int, u_int, int, int *);
extern int ImageBit8ToBit24Unit32(char **, char *, int, int);
extern void XMGetSubImageDataDepth24(char *, int, int, int, int,
                    char *, int, int);
extern int event_handler(register XImage *, register u_char *, int *);
extern void refresh_window(register XImage *);
extern int drag_image(register XImage *, register u_char *, int, int);
extern int move_image(register XImage *, register u_char *, int, int);
extern int button_release(XEvent *, register XImage *, register u_char *);
extern void button_press(XEvent *);

/* dpyio.c */
extern int readfile(char *, u_char **, int *, u_int *, u_int *, u_int *,
                    u_int *, int *);
extern int createfile(char *);
extern void unlinkfile(char *);
extern void buildheader(struct header_t *, char *, u_int, u_int, u_int,
                    u_int, int);
extern int writeheader(FILE *, struct header_t *);
extern int readheader(FILE *, struct header_t *);
extern int writedata(FILE *, u_char *, int);
extern int readdata(FILE *, u_char *, int);
extern int fdclose(int, char *);

/* dpynorm.c */
extern int dpynorm(int, char **);

/* dpypipe.c */
extern int pipecomm(int, char **);
extern int pipe_parent(register FILE *);
extern int pipe_child(int, char **, register FILE *);

/* dpytmp.c */
extern int tmpcomm(int, char **);
extern int tmp_parent(int);
extern int tmp_child(int, char **, int);

/* tally.c */
extern int bitcount(register u_int);
extern void bytecount(register u_char *, register u_long, register u_long *);
extern void pixelcount(register u_char *, register u_long,
                       register u_long *, register u_long *);

#endif
