/************************************************************************

      PACKAGE:  PCASYS TOOLS

      FILE:     MKTRAN.C

      AUTHORS:  Craig Watson
                cwatson@nist.gov
                G. T. Candela
      DATE:     08/01/1995

#cat: mktran - Makes a transform matrix Psi^t * W, where Psi's columns
#cat:          are the first n_eigvecs_use eigenvectors from eigvecs_file
#cat:          and W is the diagonal matrix of the weights from regwts_file.

File formats:
  regwts_file: matrix, dims. rw x rh
  eigvecs_file: matrix, dims. n by nfeats where n is the number
    of eigenvectors contained in the file
  tranmat_file: matrix, n_eigvecs_use by nfeats.

*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ffpis/util/usagemcs.h>
#include <ffpis/util/little.h>
#include <ffpis/util/datafile.h>
#include <ffpis/util/util.h>
#include <ffpis/util/memalloc.h>


/* Makes the diagonal matrix corresponding to the regional weights,
making only the elements on the diagonal. */

static void mktran_regwts_to_w(float *regwts,int rw,int rh,float *w,int iw,int ih)
{
  int i, ii, iis, iie, j, jj, jjs, jje, k;
  float a, *w_ptr;
  int p;

  w_ptr = w + iw*(ih/2);
  for(i = iis = k = 0, iie = 2; i < rh; i++, iis += 2, iie += 2)
    for(j = jjs = 0, jje = 2; j < rw; j++, jjs += 2, jje += 2, k++)
      for(ii = iis, a = regwts[k]; ii < iie; ii++)
	for(jj = jjs; jj < jje; jj++) {
          p = ii*iw+jj;
          w[p] = w_ptr[p] = a;
        }
}

/*******************************************************************/
int main(int argc, char *argv[])
{
  char *regwts_file, *eigvecs_file, *tranmat_file,
    *tranmat_file_desc, the_tranmat_file_desc[500],
    *adesc, str[400], *ascii_outfile;
  int n_eigvecs_use, n_eigvecs_have, ascii_out=0, i, j;
  float *regwts, *eigvecs, *tranmat, *p, *q;
  static float *w;
  int rw, rh, nfeats;

  Usage("<regwts_file> <eigvecs_file> <n_eigvecs_use>\n\
<tranmat_file> <tranmat_file_desc> <ascii_outfile>");
  regwts_file = *++argv;
  eigvecs_file = *++argv;
  n_eigvecs_use = atoi(*++argv);
  tranmat_file = *++argv;
  tranmat_file_desc = *++argv;
  ascii_outfile = *++argv;
  if(!strcmp(ascii_outfile, "y"))
    ascii_out = 1;
  else if(!strcmp(ascii_outfile, "n"))
    ascii_out = 0;
  else
    fatalerr("mktran", "ascii_outfile must be y or n", NULL);

  matrix_read(regwts_file, &adesc, &rh, &rw, &regwts);
  free(adesc);

  matrix_read_dims(eigvecs_file, &n_eigvecs_have, &nfeats);
  if(rw*rh != nfeats/8) {
    sprintf(str, "#regwts (rw*rh)[%d] != #nfeats/8 [%d]", rw*rh, nfeats/8);
    fatalerr("mktran", str, NULL);
  }

  if(n_eigvecs_use > n_eigvecs_have) {
    sprintf(str, "no. of eigenvectors to use, %d, is larger than \
no. of eigenvectors, %d, contained in file %s", n_eigvecs_use,
n_eigvecs_have, eigvecs_file);
    fatalerr("mktran", str, NULL);
  }    

  matrix_read_submatrix(eigvecs_file, 0, n_eigvecs_use - 1, 0,
    nfeats-1, &adesc, &eigvecs);
  free(adesc);

  if(!(tranmat = (float *)malloc(n_eigvecs_use * nfeats *
    sizeof(float))))
    fatalerr("mktran", "malloc", "tranmat");

  malloc_flt(&w, 8*(rw*rh), "mktran w");
  mktran_regwts_to_w(regwts, rw, rh, w, 2*rw, 4*rh);

  for(i = 0, p = tranmat, q = eigvecs; i < n_eigvecs_use; i++)
    for(j = 0; j < nfeats; j++)
      *p++ = *q++ * w[j];

  free(w);
  if(!strcmp(tranmat_file_desc, "-"))
    sprintf(the_tranmat_file_desc, "Transform matrix, made by \
mktran from regional weights %s and eigenvectors %s, using first %d \
eigenvectors", regwts_file, eigvecs_file, n_eigvecs_use);
  else
    strcpy(the_tranmat_file_desc, tranmat_file_desc);
  matrix_write(tranmat_file, the_tranmat_file_desc, ascii_out,
    n_eigvecs_use, nfeats, tranmat);
  return 0;
}

/*******************************************************************/

