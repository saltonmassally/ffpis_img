/************************************************************************

      PACKAGE:  PCASYS TOOLS

      FILE:     LINTRAN.C

      AUTHORS:  Craig Watson
                cwatson@nist.gov
                G. T. Candela
      DATE:     08/01/1995

#cat: lintran - Runs a linear transform on a set of vectors.

*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ffpis/util/usagemcs.h>
#include <ffpis/util/little.h>
#include <ffpis/util/datafile.h>
#include <ffpis/util/util.h>

int main(int argc, char *argv[])
{
  FILE *fp_in, *fp_out;
  char *tranmat_file, *vecsfile_out, *vecsfile_out_desc,
    *desc, *ascii_outfile, *adesc, str[400];
  int anint, nrows_use, message_freq, ascii_out=0, tran_dim1, tran_dim2,
    nvecs, ascii_in, dim1, dim2, iarg, i, k, old_message_len = 0;
  float *tranmat, *invec, *invec_e, *outvec, *outvec_e, *p, *q, *r, a;

  if(argc < 8)
    usage("<vecsfile_in[vecsfile_in...]> <tranmat_file> <nrows_use>\n\
<vecsfile_out> <vecsfile_out_desc> <ascii_outfile> <message_freq>");
  tranmat_file = argv[argc - 6];
  nrows_use = atoi(argv[argc - 5]);
  vecsfile_out = argv[argc - 4];
  vecsfile_out_desc = argv[argc - 3];
  ascii_outfile = argv[argc - 2];
  message_freq = atoi(argv[argc - 1]);
  if(nrows_use < 1)
    fatalerr("lintran", "nrows_use must be >= 1", NULL);
  if(!strcmp(ascii_outfile, "y"))
    ascii_out = 1;
  else if(!strcmp(ascii_outfile, "n"))
    ascii_out = 0;
  else
    fatalerr("lintran", "ascii_outfile must be y or n", NULL);
  if(message_freq < 0)
    fatalerr("lintran", "message_freq must be >= 0", NULL);
  matrix_read_dims(tranmat_file, &tran_dim1, &tran_dim2);
  if(nrows_use > tran_dim1) {
    sprintf(str, "nrows_use must be <= first dim. (%d) of transform \
matrix", tran_dim1);
    fatalerr("lintran", str, NULL);
  }
  for(iarg = 1, nvecs = 0; iarg < argc - 6; iarg++) {
    matrix_read_dims(argv[iarg], &dim1, &dim2);
    nvecs += dim1;
    if(dim2 != tran_dim2) {
      sprintf(str, "second dim., %d, of vectors file %s, does not \
equal second dim., %d, of transform matrix %s", dim2, argv[iarg],
        tran_dim2, tranmat_file);
      fatalerr("lintran", str, NULL);
    }
  }
  if(!strcmp(vecsfile_out_desc, "-")) {
    if(!(desc = malloc(200 + (argc - 5) * 200)))
      fatalerr("lintran", "malloc", "desc");
    sprintf(desc, "Made by lintran, using \
transform matrix %s, from vectors file(s)", tranmat_file);
    for(iarg = 1; iarg < argc - 6; iarg++) {
      strcat(desc, " ");
      strcat(desc, argv[iarg]);
      if(iarg < argc - 7)
	strcat(desc, ",");
    }
  }
  else
    desc = vecsfile_out_desc;
  matrix_writerow_init(vecsfile_out, desc,
    ascii_out, nvecs, nrows_use, &fp_out);
  matrix_read_submatrix(tranmat_file, 0, nrows_use - 1, 0,
    tran_dim2 - 1, &adesc, &tranmat);
  free(adesc);
  if(!(invec = (float *)malloc(tran_dim2 * sizeof(float))))
    fatalerr("lintran", "malloc", "invec");
  invec_e = invec + tran_dim2;
  if(!(outvec = (float *)malloc(nrows_use * sizeof(float))))
    fatalerr("lintran", "malloc", "outvec");
  outvec_e = outvec + nrows_use;
  for(iarg = 1; iarg < argc - 6; iarg++) {
    matrix_readrow_init(argv[iarg], &adesc, &ascii_in, &dim1, &anint,
      &fp_in);
    free(adesc);
    for(k = 0; k < dim1; k++) {
      if(message_freq && !(k % message_freq)) {
	for(i = 0; i < old_message_len; i++)
	  printf("\b");
	sprintf(str, "read vector %d (of %d) of file %d (of %d)",
          k + 1, dim1, iarg, argc - 7);
	fputs(str, stdout);
	fflush(stdout);
	old_message_len = strlen(str);
      }
      matrix_readrow(fp_in, ascii_in, tran_dim2, invec);
      for(p = outvec, q = tranmat; p < outvec_e;) {
	for(a = 0., r = invec; r < invec_e;)
	  a += *q++ * *r++;
	*p++ = a;
      }
      matrix_writerow(fp_out, ascii_out, nrows_use, outvec);
    }
    fclose(fp_in);
  }
  if(message_freq)
    printf("\n");
  return 0;
}
