#ifndef _JPEGLSD4_H
#define _JPEGLSD4_H

#include <jpegl.h>

#define MAX_HUFFBITS_JPEGL_SD4 10

extern int jpegl_sd4_decode_mem(unsigned char *, const int, const int,
                 const int, const int, unsigned char *);
/*
static int getc_huffman_table_jpegl_sd4(HUF_TABLE **, unsigned char **,
                 unsigned char *);
static int decode_data_jpegl_sd4(int *, int *, int *, int *,
                 unsigned char *, unsigned char **, unsigned char *, int *);
static int getc_nextbits_jpegl_sd4(unsigned short *, unsigned char **,
                 unsigned char *, int *, const int);
*/

#endif /* !_JPEGLSD4_H */
