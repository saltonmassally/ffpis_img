/************************************************************************

      PACKAGE:  PCASYS TOOLS

      FILE:     CHGDESC.C

      AUTHORS:  Craig Watson
                cwatson@nist.gov
                G. T. Candela
      DATE:     08/01/1995

#cat: chgdesc - Given a PCASYS data file this command replaces its
#cat:           description.  The program makes a temporary copy of
#cat:           the file.

*************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <ffpis/util/usagemcs.h>
#include <ffpis/util/util.h>
#include <ffpis/util/little.h>

int main(int argc, char *argv[])
{
  FILE *fp_in, *fp_out;
  char *datafile, *new_desc, tempfile[200], *p, achar, str[400];
  int i;

  Usage("<datafile> <new_desc>");
  datafile = *++argv;
  new_desc = *++argv;
  for(p = new_desc; *p; p++)
    if(*p == '\n')
      fatalerr("chgdesc", "new description contains a newline", NULL);
  sprintf(tempfile, "%s_chgdesc%d", datafile, getpid());
  if(exists(tempfile)) {
    sprintf(str, "temporary file that must be produced, %s, already \
exists; exiting rather than clobbering that file", tempfile);
    fatalerr("chgdesc", str, NULL);
  }
  if(!(fp_out = fopen(tempfile, "wb")))
    fatalerr("chgdesc", "fopen for writing failed", tempfile);
  fprintf(fp_out, "%s\n", new_desc);
  if(!(fp_in = fopen(datafile, "rb"))) {
    fclose(fp_out);
    unlink(tempfile);
    fatalerr("chgdesc", "fopen for reading failed", datafile);
  }
  while((i = getc(fp_in)) != '\n')
    if(i == EOF) {
      fclose(fp_out);
      unlink(tempfile);
      fatalerr("chgdesc", "file ends partway through description \
field", datafile);
    }
  while(fread(&achar, sizeof(char), 1, fp_in) == 1)
    fwrite(&achar, sizeof(char), 1, fp_out);
  fclose(fp_in);
  fclose(fp_out);
  if(rename(tempfile, datafile)) {
    sprintf(str, "rename of %s to %s failed", tempfile, datafile);
    syserr("chgdesc", str, NULL);
  }
  return 0;
}
