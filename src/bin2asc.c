/************************************************************************

      PACKAGE:  PCASYS TOOLS

      FILE:     BIN2ASC.C

      AUTHORS:  Craig Watson
                cwatson@nist.gov
                G. T. Candela
      DATE:     08/01/1995

#cat: bin2asc - Reads a PCASYS binary data file of any type and
#cat:           writes a corresponding ascii data file.

*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ffpis/util/datafile.h>
#include <ffpis/util/usagemcs.h>
#include <ffpis/util/memalloc.h>
#include <ffpis/util/util.h>
#include <ffpis/util/little.h>


int main(int argc, char *argv[])
{
  FILE *fp_in;
  char *binary_data_in, *ascii_data_out, file_type, asc_or_bin,
    codes_line[5];
  static char desc[DESC_DIM], *desc2;
  int i, j, dim1, dim2;
  float *the_floats;
  char **long_classnames;
  unsigned char *the_classes;

  Usage("<binary_data_in> <ascii_data_out>");
  binary_data_in = argv[1];
  ascii_data_out = argv[2];
  if(!(fp_in = fopen(binary_data_in, "rb")))
    fatalerr("bin2asc", "fopen for reading failed", binary_data_in);
  for(i = 0; i < DESC_DIM && (j = getc(fp_in)) != '\n'; i++) {
    if(j == EOF)
      fatalerr("bin2asc", "input ends partway through description \
field", binary_data_in);
    desc[i] = j;
  }
  if(i == DESC_DIM)
    fatalerr("bin2asc", "description too long", binary_data_in);
  desc[i] = 0;
  fgets(codes_line, 5, fp_in);
  sscanf(codes_line, "%c %c", &file_type, &asc_or_bin);
  if(asc_or_bin != PCASYS_BINARY_FILE)
    fatalerr("bin2asc", "not a PCASYS ascii file", binary_data_in);
  fclose(fp_in);

  if(file_type == PCASYS_MATRIX_FILE) {
    matrix_read(binary_data_in, &desc2, &dim1, &dim2, &the_floats);
    matrix_write(ascii_data_out, desc2, 1, dim1, dim2, the_floats);
    free(the_floats);
    free(desc2);
  }
  else if (file_type == PCASYS_COVARIANCE_FILE) {
    covariance_read(binary_data_in, &desc2, &dim1, &dim2, &the_floats);
    covariance_write(ascii_data_out, desc2, 1, dim1, dim2, the_floats);
    free(the_floats);
    free(desc2);
  }
  else if(file_type == PCASYS_CLASSES_FILE) {
    classes_read_ind(binary_data_in, &desc2, &dim1, &the_classes, &dim2,
                     &long_classnames);
    classes_write_ind(ascii_data_out, desc2, 1, dim1, the_classes,
                      dim2, long_classnames);
    free(the_classes);
    free(desc2);
    free_dbl_char(long_classnames, dim2);
  }
  else
    fatalerr("bin2asc", "illegal file type code", binary_data_in);

  exit(0);
}
