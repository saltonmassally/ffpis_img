#include <sys/types.h>
extern void set_logical_bit(void);
extern void copy_bits(void);
extern void copy_bytes(void);
extern void copy_image(
		unsigned char *frdata, int frx, int fry, int frw,int frh,
		unsigned char *todata, int tox, int toy, int tow,int toh,
		int cpw, int cph);
extern void copy_logical_bits(void);
/* FIXME logop can only be copy not LOG_AND, LOG_XOR, etc */
void copy_logical_image(
		int logop,
		unsigned char *frdata, int frx, int fry, int frw,int frh,
		unsigned char *todata, int tox, int toy, int tow,int toh,
		int cpw, int cph);
void binary_fill_partial( int op, u_char *src,int  i,u_char  *dst,
		int  j, int  bits);
int binary_image_mpad( u_char **image,
		u_int *w,u_int  *h,u_int  mpadw,u_int  mpadh,u_int bg);
int binary_image_pad( u_char **image,
		u_int w,u_int  h,u_int  padw,u_int  padh, int bg);

void binary_subimage_copy ( u_char *src, int  srcw,int srch, u_char  *dst,
	       	int dstw, int dsth, int srcx,int srcy, int  cpw,int cph,
		int dstx,int dsty);
void binary_subimage_copy_8 ( u_char *src, int  srcw,int srch,
		u_char  *dst, int dstw, int dsth, int srcx,int srcy,
		int  cpw,int cph, int dstx,int dsty);
void binary_subimage_copy_gt ( u_char *src, int  srcw,int srch,
		u_char  *dst, int dstw, int dsth, int srcx,int srcy,
		int  cpw,int cph, int dstx,int dsty);
void binary_subimage_copy_lt ( u_char *src, int  srcw,int srch,
		u_char  *dst, int dstw, int dsth, int srcx,int srcy,
		int  cpw,int cph, int dstx,int dsty);
void binary_subimage_copy_eq ( u_char *src, int  srcw,int srch,
		u_char  *dst, int dstw, int dsth, int srcx,int srcy,
		int  cpw,int cph, int dstx,int dsty);
