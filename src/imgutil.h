
float PixPerByte(int depth);
int SizeFromDepth(int pixwidth,int pixheight,int depth);
int WordAlignFromDepth(int pixwidth,int  depth);
void init_image_data( unsigned char **data, int *size,
		int width,int  height,int  depth);
unsigned char *allocate_image( int width,int  height,int  depth);
unsigned char *mallocate_image( int width,int  height,int  depth);
short *alloc_short_image( int width,int  height);
unsigned char *alloc_char_image( int width,int  height);
int *alloc_int_image( int width,int  height);
int allocate_aligned_image( unsigned char **adata, int *awidth,
		                int  width,int  height,int  depth);
int width_16(int width);
int WordAlignImage(
		unsigned char **adata,
		int *awidth,int *alength,
		unsigned char  *data,
		int width,int height,int depth
		);
