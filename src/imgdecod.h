#ifndef _IMGDECOD_H
#define _IMGDECOD_H

#define IMG_IGNORE  2

extern int read_and_decode_dpyimage(char *, int *, unsigned char **, int *,
                                    int *, int *, int *, int *);

extern int read_and_decode_image(char *, int *, unsigned char **, int *,
                                 int *, int *, int *, int *, int *,
                                 int *, int *, int *);

extern int ihead_decode_mem(unsigned char **, int *, int *, int *,
                            int *, int *, unsigned char *, const int);
void rldecomp(unsigned char *indata,int inbytes,unsigned char *outdata,
		                int *outbytes, int outsize);
void rlcomp( unsigned char *indata,
		int inbytes,
		unsigned char  *outdata,
		int *outbytes,int outsize);

#endif /* !_IMGDECOD_H */
