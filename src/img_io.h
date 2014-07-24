#ifndef _IMG_IO_H
#define _IMG_IO_H

#include <ihead.h>

extern int read_raw_from_filesize(char *, unsigned char **, int *);
extern int write_raw_from_memsize(char *, unsigned char *, const int);
extern int read_raw_or_ihead(const int, char *, IHEAD **,
                             unsigned char **, int *, int *, int *);
extern int read_raw_or_ihead_wsq(const int, char *, IHEAD **,
                             unsigned char **, int *, int *, int *);
int write_raw_or_ihead(const int, char *, unsigned char *,
                       const int, const int, const int, const int);

#endif /* !_IMG_IO_H */
