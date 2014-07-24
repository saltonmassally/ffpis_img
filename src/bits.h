/************************************************************/
/*         File Name: Bits.h                                */
/*         Package:   NIST Bit Manipulation Macros          */
/*         Author:    Michael D. Garris                     */
/*         Date:      9/27/89                               */
/************************************************************/

#define LOG_COPY	0
#define LOG_OR		1
#define LOG_AND		2
#define LOG_XOR		3

#define get_bit(_b,_i) (((int)((_b) & bit_masks[(_i)]) > 0) ? (1) : (0))

#define set_bit(_b,_i,_p) \
if((_p) == 0) \
  *(_b) = (*(_b)) & (~bit_masks[(_i)]); \
else \
  *(_b) = (*(_b)) | bit_masks[(_i)];
