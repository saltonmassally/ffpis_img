#ifndef _MEMALLOC_H
#define _MEMALLOC_H

extern void free_dbl_char(char **, const int);
extern void free_dbl_flt(float **, const int);
extern void malloc_dbl_char(char ***, const int, const int, char *);
extern void malloc_dbl_flt(float ***, const int, const int, char *);
extern void malloc_flt(float **, const int, char *);

void malloc_char(char **ptr,int n,char * s);
void malloc_uchar(unsigned char **ptr,int  n,char * s);
void malloc_shrt(short **ptr, short n,char *s);
void malloc_int(int **ptr, int n,char * s);
void malloc_flt(float **ptr,int n,char * s);
void calloc_char( char **ptr, int n, char *s);
void calloc_uchar( unsigned char **ptr, int n, char *s);
void calloc_shrt( short **ptr, short n, char *s);
void calloc_int( int **ptr, int n, char *s);
void calloc_flt( float **ptr, int n, char *s);
void malloc_dbl_char_l1( char ***ptr, int ndbl, char *s);
void malloc_dbl_uchar_l1( unsigned char ***ptr, int ndbl, char *s);
void malloc_dbl_shrt_l1( short ***ptr, int ndbl, char *s);
void malloc_dbl_int_l1( int ***ptr, int ndbl, char *s);
void malloc_dbl_flt_l1( float ***ptr, int ndbl, char *s);
void realloc_char( char **ptr, int n, char *s);
void realloc_uchar( unsigned char **ptr, int n, char *s);
void realloc_shrt( short **ptr, int n, char *s);
void realloc_int( int **ptr, int n, char *s);
void realloc_flt( float **ptr, int n, char *s);
void realloc_dbl_int_l1( int ***ptr, int ndbl, char *s);
void realloc_dbl_char_l1( char ***ptr, int ndbl, char *s);
void realloc_dbl_uchar_l1( unsigned char ***ptr, int ndbl, char *s);
void realloc_dbl_flt_l1( float ***ptr, int ndbl, char *s);
void free_dbl_char(char **ptr, const int n);
void free_dbl_flt(float **ptr, const int n);
void malloc_dbl_char(char ***ptr, const int ndbl, const int n, char *s);
void malloc_dbl_flt(float ***ptr, const int ndbl, const int n, char *s);
#endif /* !_MEMALLOC_H */
