/************************************************************************

      PACKAGE:  PCASYS TOOLS

      FILE:     MLPFEATS.C

      AUTHORS:  Craig Watson
                cwatson@nist.gov
      DATE:     04/06/2001

#cat: mlpfeats - Converts a feature/class file set from PCASYS to the MLP
#cat:            feature file format.

*************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <ffpis/util/datafile.h>
#include <swap.h>
#include <ffpis/util/memalloc.h>
#include <ffpis/util/swapbyte.h>
#include <ffpis/util/util.h>

int main(int argc, char *argv[])
{
   FILE *pout;
   char *feats_file, *cls_file, *mlp_file;
   float *feats, *cls_targs;
   char **classes, *feats_desc, *cl_desc;
   int nfeats, npats, ncls, nouts;
   unsigned char *cls_ids;
   int i;
   int itmp, itmp2;


   if (argc != 4) {
      fprintf(stderr,
         "Usage: %s <feats_file> <class_file> <mlp_feats_file>\n\n", argv[0]);
      exit(-1);
   }

   feats_file = argv[1];
   cls_file = argv[2];
   mlp_file = argv[3];

   matrix_read(feats_file, &feats_desc, &npats, &nfeats, &feats);
#ifdef __i386__
   swap_float_bytes_vec(feats, npats*nfeats);
#endif
   fprintf(stdout, "npats = %d\nnfeats = %d\n", npats, nfeats);

   classes_read_ind(cls_file, &cl_desc, &ncls, &cls_ids, &nouts, &classes);

   fprintf(stdout, "nouts = %d\nncls = %d\n", nouts, ncls);
   for(i = 0; i < nouts; i++)
      fprintf(stdout, "%s ", classes[i]);
   fprintf(stdout, "\n");

   if(ncls != npats) {
      fprintf(stderr,
         "# classes (%d) in class file != # patterns (%d) in feats file\n",
          ncls, npats);
      free(feats);
      free(cls_ids);
      free_dbl_char(classes, nouts);
      exit(-1);
   }

   cls_targs = (float *)calloc(ncls * nouts, sizeof(float));
   if(cls_targs == (float *)NULL) {
      free(feats);
      free(cls_ids);
      free_dbl_char(classes,nouts);
      syserr(argv[0], "calloc", "cls_targs");
   }

   for(i = 0; i < ncls; i++)
      cls_targs[i*nouts+cls_ids[i]] = 1.0;
   free(cls_ids);
#ifdef __i386__
   swap_float_bytes_vec(cls_targs, ncls*nouts);
#endif

   if((pout = fopen(mlp_file, "wb")) == NULL) {
      free(feats);
      free_dbl_char(classes,nouts);
      free(cls_targs);
      syserr(argv[0],"fopen","out_infile");
   }

/* write npats, nfeats, nouts and three unused zeros */
   itmp = 24;
#ifdef __i386__
   swap_int_bytes(itmp);
   swap_int_bytes(npats);
   swap_int_bytes(nfeats);
   swap_int_bytes(nouts);
#endif
   fwrite(&itmp, sizeof(int), 1, pout);
   fwrite(&npats, sizeof(int), 1, pout);
   fwrite(&nfeats, sizeof(int), 1, pout);
   fwrite(&nouts, sizeof(int), 1, pout);
   itmp2 = 0;
   fwrite(&itmp2, sizeof(int), 1, pout);
   fwrite(&itmp2, sizeof(int), 1, pout);
   fwrite(&itmp2, sizeof(int), 1, pout);
   fwrite(&itmp, sizeof(int), 1, pout);
#ifdef __i386__
   swap_int_bytes(npats);
   swap_int_bytes(nfeats);
   swap_int_bytes(nouts);
#endif

   itmp = 32 * (nouts);
#ifdef __i386__
   swap_int_bytes(itmp);
#endif
   fwrite(&itmp, sizeof(int), 1, pout);
   for(i = 0; i < nouts; i++)
         fwrite(classes[i], sizeof(char), 32, pout);
   fwrite(&itmp, sizeof(int), 1, pout);
   free_dbl_char(classes,nouts);


   itmp = sizeof(float) * nfeats;
#ifdef __i386__
   swap_int_bytes(itmp);
#endif
    itmp2 = sizeof(float) * (nouts);
#ifdef __i386__
    swap_int_bytes(itmp2);
#endif
   for(i = 0; i < npats; i++) {
         fwrite(&itmp, sizeof(int), 1, pout);
         fwrite(&(feats[i*nfeats]), sizeof(float), nfeats, pout);
         fwrite(&itmp, sizeof(int), 1, pout);

         fwrite(&itmp2, sizeof(int), 1, pout);
         fwrite(&(cls_targs[i*nouts]), sizeof(float), nouts, pout);
         fwrite(&itmp2, sizeof(int), 1, pout);
   }
   free(cls_targs);
   free(feats);

   fclose(pout);
   return 0;
}
