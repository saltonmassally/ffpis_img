#ifndef _JPEGB_H
#define _JPEGB_H

#include <jpeglib.h>
//#include <jerror.h>
//#include <jconfig.h>

#ifndef _NISTCOM_H
#include <nistcom.h>
#endif

#define SOI   0xffd8
#define EOI   0xffd9
#define SOS   0xffda
#define SOF0  0xffc0
#define COM   0xfffe
/* Case for getting ANY marker. */
#define ANY   0xffff

/* encoder.c */
extern int jpegb_encode_mem(unsigned char **, int *, const int,
                            unsigned char *, const int, const int,
                            const int, const int, char *);
extern int jpegb_encode_file(FILE *, const int,
                             unsigned char *, const int, const int,
                             const int, const int, char *);

/* decoder.c */
extern int jpegb_decode_mem(unsigned char **, int *, int *, int *, int *,
                            int *, unsigned char *, const int);
extern int jpegb_decode_file(unsigned char **, int *, int *, int *, int *,
                             int *, FILE *);

/* marker.c */
extern int read_marker_jpegb(unsigned short *, const int, FILE *);
extern int getc_marker_jpegb(unsigned short *, const int,
                             unsigned char **, unsigned char *);
extern int put_nistcom_jpegb(j_compress_ptr, char *, const int, const int,
                             const int, const int, const int, const int);
extern int read_nistcom_jpegb(NISTCOM **, FILE *);
extern int getc_nistcom_jpegb(NISTCOM **, unsigned char *, const int);


/* membuf.c */
extern void jpeg_membuf_dest(j_compress_ptr, JOCTET *, size_t);
extern void jpeg_membuf_src(j_decompress_ptr, JOCTET *, size_t);

/* ppi.c */
extern int get_ppi_jpegb(int *, j_decompress_ptr);

#endif /* !_JPEGB_H */
