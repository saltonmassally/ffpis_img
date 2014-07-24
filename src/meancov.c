/************************************************************************

      PACKAGE:  PCASYS TOOLS

      FILE:     MEANCOV.C

      AUTHORS:  Craig Watson
                cwatson@nist.gov
                G. T. Candela
      DATE:     08/01/1995

#cat: meancov - Computes the mean vector and covariance matrix
#cat:           for a set of feature vectors.

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
  FILE *fp;
  char *meanfile_out, *meanfile_out_desc, *the_meanfile_out_desc,
    *covfile_out, *covfile_out_desc, *the_covfile_out_desc,
    str[500], *cjunk, *ascii_outfiles;
  int anint, ascii_out=0, ascii_in, message_freq, dim1, dim2, a_dim2,
    iarg, old_message_len = 0, i, j, k, nvecs = 0;
  float *cov, *covp, *mean, *meanpe, *meanp, *meanp2, meanelt, *v,
    *vp, velt, *vp2, *vp2f;

  if(argc < 8)
    usage("<vecsfile_in[vecsfile_in...]> <meanfile_out>\n\
<meanfile_out_desc> <covfile_out> <covfile_out_desc> <ascii_outfiles>\n\
<message_freq>");
  meanfile_out = argv[argc - 6];
  meanfile_out_desc = argv[argc - 5];
  covfile_out = argv[argc - 4];
  covfile_out_desc = argv[argc - 3];
  ascii_outfiles = argv[argc - 2];
  message_freq = atoi(argv[argc - 1]);
  if(!strcmp(ascii_outfiles, "y"))
    ascii_out = 1;
  else if(!strcmp(ascii_outfiles, "n"))
    ascii_out = 0;
  else
    fatalerr("meancov", "ascii_outfiles must be y or n", NULL);
  if(message_freq < 0)
    fatalerr("meancov", "message_freq must be >= 0", NULL);

  matrix_read_dims(argv[1], &anint, &dim2);
  if(argc > 8) { /* Several input files; check that all have same
    second dimension. */
    if(message_freq)
      printf("checking that all input matrices have same second \
dimension\n");
    for(iarg = 2; iarg < argc - 6; iarg++) {
      matrix_read_dims(argv[iarg], &anint, &a_dim2);
      if(a_dim2 != dim2) {
	sprintf(str, "second dim., %d, of input matrix %s, does \
not equal second dim., %d, of first input matrix %s", a_dim2,
          argv[iarg], dim2, argv[1]);
	fatalerr("meancov", str, NULL);
      }
    }
  }

  /* Accumulate stuff for mean and covariance.  Nonstrict lower
  triangle of covariance is sufficient, since it is symmetric. */
  if(!(cov = (float *)calloc((dim2 * (dim2 + 1)) / 2,
    sizeof(float))))
    fatalerr("meancov", "calloc", "cov");
  if(!(mean = (float *)calloc(dim2, sizeof(float))))
    fatalerr("meancov", "calloc", "mean");
  meanpe = mean + dim2;
  if(!(v = (float *)malloc(dim2 * sizeof(float))))
    fatalerr("meancov", "malloc", "v");
  for(iarg = 1; iarg < argc - 6; iarg++) {
    matrix_readrow_init(argv[iarg], &cjunk, &ascii_in, &dim1,
      &anint, &fp);
    free(cjunk);
    for(k = 0; k < dim1; k++) {
      if(message_freq && !(k % message_freq)) {
	for(i = 0; i < old_message_len; i++)
	  printf("\b");
	sprintf(str, "accumulating from vector %d (of %d) of file \
%d (of %d)", k + 1, dim1, iarg, argc - 7);
	fputs(str, stdout);
	fflush(stdout);
	old_message_len = strlen(str);
      }
      matrix_readrow(fp, ascii_in, dim2, v);
      for(meanp = mean, vp2f = vp = v, covp = cov; meanp < meanpe;
        vp2f++) {
	*meanp++ += (velt = *vp++);
	for(vp2 = v; vp2 <= vp2f;)
	  *covp++ += velt * *vp2++;
      }
    }
    fclose(fp);
    nvecs += dim1;
  }

  if(message_freq)
    printf("\nfinishing mean\n");
  for(meanp = mean; meanp < meanpe;)
    *meanp++ /= nvecs;

  printf("finishing covariance\n");
  for(i = 0, meanp = mean, covp = cov; i < dim2; i++, meanp++)
    for(j = 0, meanelt = *meanp, meanp2 = mean; j <= i; j++, meanp2++,
      covp++)
      *covp = *covp / nvecs - meanelt * *meanp2;

  if(!strcmp(meanfile_out_desc, "-")) {
    if(!(the_meanfile_out_desc = malloc(strlen("Mean vector, \
made by meancov from") + (argc - 7) * 200)))
      fatalerr("meancov", "malloc", "the_meanfile_out_desc");
    strcpy(the_meanfile_out_desc, "Mean vector, made by meancov from");
    for(iarg = 1; iarg <= argc - 7; iarg++) {
      strcat(the_meanfile_out_desc, " ");
      strcat(the_meanfile_out_desc, argv[iarg]);
      if(iarg < argc - 7)
	strcat(the_meanfile_out_desc, ",");
    }
  }
  else
    the_meanfile_out_desc = meanfile_out_desc;
  if(!strcmp(covfile_out_desc, "-")) {
    if(!(the_covfile_out_desc = malloc(strlen("Covariance, \
made by meancov from") + (argc - 7) * 200)))
      fatalerr("meancov", "malloc", "the_covfile_out_desc");
    strcpy(the_covfile_out_desc, "Covariance, made by meancov from");
    for(iarg = 1; iarg <= argc - 7; iarg++) {
      strcat(the_covfile_out_desc, " ");
      strcat(the_covfile_out_desc, argv[iarg]);
      if(iarg < argc - 7)
	strcat(the_covfile_out_desc, ",");
    }
  }
  else
    the_covfile_out_desc = covfile_out_desc;
  if(message_freq)
    printf("writing\n");
  matrix_write(meanfile_out, the_meanfile_out_desc, ascii_out, 1,
    dim2, mean);
  covariance_write(covfile_out, the_covfile_out_desc, ascii_out,
    dim2, nvecs, cov);
  return 0;
}
