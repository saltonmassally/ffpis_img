#ifndef _IMGTYPE_H
#define _IMGTYPE_H

#include <wsq.h>
#include <jpegb.h>
#include <jpegl.h>
#include <ihead.h>

#define UNKNOWN_IMG -1
#define RAW_IMG     0
#define WSQ_IMG     1
#define JPEGL_IMG   2
#define JPEGB_IMG   3
#define IHEAD_IMG   4

/* imgtype.c */
extern int image_type(int *, unsigned char *, const int);
extern int jpeg_type(int *, unsigned char *, const int);

#endif /* !_IMGTYPE_H */
