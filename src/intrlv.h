#ifndef _INTRLV_H
#define _INTRLV_H

#ifndef MAX_CMPNTS
#define MAX_CMPNTS   4
#endif


extern int intrlv2not_mem(unsigned char **, int *, unsigned char *,
                   const int, const int, const int, int *, int *, const int);

extern int not2intrlv2not_mem(unsigned char **, int *, unsigned char *,
                   const int, const int, const int, int *, int *, const int);
extern int not2intrlv_mem(unsigned char **oodata, int *oolen,
		unsigned char *idata, const int width, const int height,
		const int depth, int *hor_sampfctr, int *vrt_sampfctr,
		const int n_cmpnts);

extern void compute_component_padding(int *, int *, const int, const int,
                   int *, int *, int *, int *, const int);

extern int pad_component_planes(unsigned char *, int *, int *, int *,
                   int *, int *, int *, int *, const int);

extern int test_image_size(const int, const int, const int, int *, int *,
                   const int, const int);

#endif /* !_INTRLV_H */
