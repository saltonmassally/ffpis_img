#ifndef _DATA_IO_H
#define _DATA_IO_H

/* dataio.c */
extern int read_byte(unsigned char *, FILE *);
extern int getc_byte(unsigned char *, unsigned char **, unsigned char *);
extern int getc_bytes(unsigned char **, const int, unsigned char **, 
                 unsigned char *);
extern int write_byte(const unsigned char, FILE *);
extern int putc_byte(const unsigned char, unsigned char *, const int, int *);
extern int putc_bytes(unsigned char *, const int, unsigned char *,
                 const int, int *);
extern int read_ushort(unsigned short *, FILE *);
extern int getc_ushort(unsigned short *, unsigned char **, unsigned char *);
extern int write_ushort(unsigned short, FILE *);
extern int putc_ushort(unsigned short, unsigned char *, const int, int *);
extern int read_uint(unsigned int *, FILE *);
extern int getc_uint(unsigned int *, unsigned char **, unsigned char *);
extern int write_uint(unsigned int, FILE *);
extern int putc_uint(unsigned int, unsigned char *, const int, int *);
extern void write_bits(unsigned char **, const unsigned short, const short,
                 int *, unsigned char *, int *);
extern void flush_bits(unsigned char **, int *, unsigned char *, int *);
extern int read_ascii_file(char *, char **);

#endif /* !_DATA_IO_H */
