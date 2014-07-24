#ifndef _SUNRAST_H
#define _SUNRAST_H

/************************************************************/
/*         File Name: Sunrast.h                             */
/*         Package:   Sun Rasterfile I/O                    */
/*         Author:    Michael D. Garris                     */
/*         Date:      8/19/99                               */
/*                                                          */
/************************************************************/

/* Contains header information related to Sun Rasterfile images. */

typedef struct sunrasterhdr {
	int	magic;		/* magic number */
	int	width;		/* width (in pixels) of image */
	int	height;		/* height (in pixels) of image */
	int	depth;		/* depth (1, 8, or 24 bits) of pixel */
	int	raslength;	/* length (in bytes) of image */
	int	rastype;	/* type of file; see SUN_* below */
	int	maptype;	/* type of colormap; see MAP_* below */
	int	maplength;	/* length (bytes) of following map */
	/* color map follows for maplength bytes, followed by image */
} SUNHEAD;

#define	SUN_MAGIC	0x59a66a95

	/* Sun supported ras_type's */
#define SUN_STANDARD	1	/* Raw pixrect image in 68000 byte order */
#define SUN_RUN_LENGTH	2	/* Run-length compression of bytes */
#define SUN_FORMAT_RGB	3	/* XRGB or RGB instead of XBGR or BGR */
#define SUN_FORMAT_TIFF	4	/* tiff <-> standard rasterfile */
#define SUN_FORMAT_IFF	5	/* iff (TAAC format) <-> standard rasterfile */

	/* Sun supported maptype's */
#define MAP_RAW		2
#define MAP_NONE	0	/* maplength is expected to be 0 */
#define MAP_EQUAL_RGB	1	/* red[maplength/3],green[],blue[] */

/*
 * NOTES:
 *   Each line of a bitmap image should be rounded out to a multiple
 *   of 16 bits.
 */

extern int ReadSunRaster(const char *ifile, SUNHEAD **osunhead,
		unsigned char **ocolormap, int *omaplen,
		unsigned char **odata, int *oscan_w, int *oimg_w,
		int *oimg_h, int *oimg_d);
extern int WriteSunRaster(char *ofile, unsigned char *data,
		const int width, const int height, const int depth);
#endif
