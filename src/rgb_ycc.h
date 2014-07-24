#ifndef _RGB_YCC_H
#define _RGB_YCC_H

#ifndef MAX_CMPNTS
#define MAX_CMPNTS   4
#endif

#ifndef sround
#define sround(x) ((int) (((x)<0) ? (x)-0.5 : (x)+0.5))
#endif

extern int rgb2ycc_mem(unsigned char **, int *, unsigned char *,
                       const int, const int, const int, const int);
extern int rgb2ycc_intrlv_mem(unsigned char **, int *, unsigned char *,
                       const int, const int, const int);
extern int rgb2ycc_nonintrlv_mem(unsigned char **, int *, unsigned char *,
                       const int, const int, const int);
extern int downsample_cmpnts(unsigned char **, int *, unsigned char *,
                       const int, const int, const int,
                       int *, int *, const int);
extern void window_avr_plane(unsigned char *, int *, int *, const int,
                       const int, unsigned char *, const int, const int);
extern int avr_window(unsigned char *, const int, const int, const int,
                       const int);
extern int ycc2rgb_mem(unsigned char **, int *, unsigned char *,
                       const int, const int, const int, const int);
extern int ycc2rgb_intrlv_mem(unsigned char **, int *, unsigned char *,
                       const int, const int, const int);
extern int ycc2rgb_nonintrlv_mem(unsigned char **, int *, unsigned char *,
                       const int, const int, const int);
extern int upsample_cmpnts(unsigned char **, int *, unsigned char *,
                       const int, const int, const int,
                       int *, int *, const int);
extern void window_fill_plane(unsigned char *, const int, const int,
                       const int, const int,
                       unsigned char *, const int, const int);
extern void fill_window(const unsigned char, unsigned char *,
                       const int, const int, const int, const int);
extern int test_evenmult_sampfctrs(int *, int *, int *, int *, const int);

#endif /* !_RGB_YCC_H */
