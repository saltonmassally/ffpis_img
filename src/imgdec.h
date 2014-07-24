#ifndef _IMGDEC_H
#define _IMGDEC_H

#ifndef _NISTCOM_H
#include <nistcom.h>
#endif

extern int read_and_decode_pcasys(char *, unsigned char **, int *,
                int *, int *, char *);
extern int get_nistcom_class(NISTCOM *, char *);
extern int get_sd_ihead_class(IHEAD *, char *);
extern void get_class_id(const char, char *, unsigned char *);

#endif /* !_IMGDEC_H */
