/************************************************************************

      PACKAGE:  PCASYS TOOLS

      FILE:     OPTOSF.C

      AUTHORS:  Craig Watson
                cwatson@nist.gov
                G. T. Candela
      DATE:     08/01/1995

#cat: optosf - Optimizes the overall smoothing factor (osf) which the
#cat:          Probabilistic Neural Net (PNN) classifier is to use.

Optimizes the overall smoothing factor (osf) which the
Probabilistic Neural Net (PNN) classifier is to use.  The idea here is
to let the prototypes set be the full set of prototypes to be used by
the classifier (already incorporating the optimized regional weights)
and to let the "tuning" set that is classified using these prototypes
to produce an activation error rate, be a smaller subset of (top)
feature vectors.  Each time a tuning vector is classified, it is left
out of the prototypes set.  (Otherwise, classication would presumably
do very well regardless of the osf, so the program would not learn a
good osf.)  It is important that the prototypes set be the full set to
be used in the finished classifier and not just a subset, since the
optimal osf depends on the number of prototypes; but the subset of it
used as the tuning set can be of any reasonable size.

The program makes a text file that has a line for each osf value
tried.  The line shows the osf, the resulting activation error rate,
and the resulting classification error rate.  The activation error
rate, which is the quantity optosf tries to minimize, is the average,
over the tuning set, of the squared difference between 1 and the
normalized PNN activation of the actual class.  The classification
error rate is the fraction of the tuning prints misclassified.  When
the program finishes, the last line of the output file will correspond
to the optimal osf that it finds.  The output file has a description
string before the osf lines; this is either outfile_desc (with newline
appended) or, if outfile_desc is - (hyphen), then the program makes a
reasonable description, which indicates the values of the important
parameters.  If verbose is y, the program writes each computed (osf,
activ. error, classif. error) to the standard output.

The optimal osf found by optosf should be specified in the parameter
file, when running the finished classifier.

*************************************************************************/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <ffpis/util/little.h>
#include <ffpis/util/usagemcs.h>
#include <ffpis/util/datafile.h>
#include <ffpis/util/memalloc.h>
#include <ffpis/util/table.h>
#include <ffpis/util/util.h>

static FILE *fp_out;
static int verbose_int;

static struct {
  char n_feats_use, osf_init, osf_initstep, osf_stepthr, tablesize,
    verbose, fvs_file, classes_file, n_fvs_use_as_protos_set,
    n_fvs_use_as_tuning_set, outfile, outfile_desc;
} setflags;

/********************************************************************/

/* Computes PNN activation error rate and classification error rate
when a set of (KL) feature vectors is classified.

Inputs:

  n_feats_use: how many features

  n_fvs_use_as_protos_set: how many feature vectors in prototypes set.

  n_fvs_use_as_tuning_set: how many feature vectors in "tuning" set,
    i.e., set that is classified to produce activation error rate.
    The proto and tuning sets start at same vector.  Each time
    a tuning vector is classified, it is left out of the prototypes
    set.

  fvs: the feature vectors, of which the first n_fvs_use_as_protos_set
    are used as the protos, and the first n_fvs_use_as_tuning_set are
    used as the tuning set.

  classes: the classes of the feature vectors

  fac: smoothing factor, really osf (overall smoothing factor) here

Outputs:

  acerror: activation error rate, i.e., average, over tuning set,
    of squared difference between 1 and the normalized activation
    of the actual class.

  classerror: classification error rate, i.e., fraction of the
    tuning set that is misclassified.
*/

static void
optosf_pnn(int n_feats_use,int  n_fvs_use_as_protos_set,
 int  n_fvs_use_as_tuning_set,float  *fvs,unsigned char *classes,int  n_cls,
 float  fac,float  *acerror,
 float *classerror)
{
  unsigned char *tu_classes_p, *pr_classes_p, hypclass;
  int itu, ipr, i, nwrong;
  float accm, *tu_p, *pr_p, *p, *pe, *q, *ac, acsum, maxac,
    anac, a, sd;

  accm = 0.;
  nwrong = 0;
  malloc_flt(&ac, n_cls, "optosf_pnn ac");
  for(itu = 0, tu_p = fvs, tu_classes_p = classes;
    itu < n_fvs_use_as_tuning_set; itu++, tu_p += n_feats_use,
    tu_classes_p++) {

    if(!(itu % 10))
      printf("itu %d\n", itu);

    memset(ac, 0, n_cls * sizeof(float));
    for(ipr = 0, pr_p = fvs, pr_classes_p = classes;
      ipr < n_fvs_use_as_protos_set; ipr++, pr_p += n_feats_use,
      pr_classes_p++) {
      if(itu == ipr)
	continue;
      for(sd = 0., pe = (p = tu_p) + n_feats_use, q = pr_p;
        p < pe; p++, q++) {
	a = *p - *q;
	sd += a * a;
      }
      ac[*pr_classes_p] += exp((double)(-fac * sd));
    }
    for(acsum = maxac = ac[0], hypclass = 0, i = 1; i < n_cls;
      i++) {
      acsum += (anac = ac[i]);
      if(anac > maxac) {
	maxac = anac;
	hypclass = i;
      }
    }
    if(acsum > 0.) {
      a = 1. - ac[*tu_classes_p] / acsum;
      accm += a * a;
    }
    else
      accm += 1.;
    if(hypclass != *tu_classes_p)
      nwrong++;
  }
  *acerror = accm / n_fvs_use_as_tuning_set;
  *classerror = (float)nwrong / n_fvs_use_as_tuning_set;
  free(ac);
}

/********************************************************************/

/* Reads an optosf parms file. */

static void
optosf_read_parms(char parmsfile[], int *n_feats_use,float *osf_init,
	float *osf_initstep,
    float *osf_stepthr,int  *tablesize,int  *verbose_int,
    char fvs_file[],char classes_file[],
    int *n_fvs_use_as_protos_set,int  *n_fvs_use_as_tuning_set, char outfile[],
    char outfile_desc[])
/* char parmsfile[], fvs_file[], classes_file[], outfile[],
  outfile_desc[];
int *n_feats_use, *tablesize, *verbose_int, *n_fvs_use_as_protos_set,
  *n_fvs_use_as_tuning_set;
float *osf_init, *osf_initstep, *osf_stepthr; */
{
  FILE *fp;
  char str[1000], *p, name_str[50], val_str[1000];

  fp = fopen_ch(parmsfile, "rb");
  while(fgets(str, 1000, fp)) {
    p = strchr(str, '#');
    if(p)
      *p = 0;
    if(sscanf(str, "%s %s", name_str, val_str) < 2)
      continue;

    if(!strcmp(name_str, "n_feats_use")) {
      *n_feats_use = atoi(val_str);
      setflags.n_feats_use = 1;
    }
    else if(!strcmp(name_str, "osf_init")) {
      *osf_init = atof(val_str);
      setflags.osf_init = 1;
    }
    else if(!strcmp(name_str, "osf_initstep")) {
      *osf_initstep = atof(val_str);
      setflags.osf_initstep = 1;
    }
    else if(!strcmp(name_str, "osf_stepthr")) {
      *osf_stepthr = atof(val_str);
      setflags.osf_stepthr = 1;
    }
    else if(!strcmp(name_str, "tablesize")) {
      *tablesize = atoi(val_str);
      setflags.tablesize = 1;
    }
    else if(!strcmp(name_str, "verbose")) {
      if(!strcmp(val_str, "y"))
	*verbose_int = 1;
      else if(!strcmp(val_str, "n"))
	*verbose_int = 0;
      else
	fatalerr("optosf_read_parms (file optosf.c)", "verbose is \
neither y nor n", NULL);
      setflags.verbose = 1;
    }
    else if(!strcmp(name_str, "fvs_file")) {
      strcpy(fvs_file, val_str);
      setflags.fvs_file = 1;
    }
    else if(!strcmp(name_str, "classes_file")) {
      strcpy(classes_file, val_str);
      setflags.classes_file = 1;
    }
    else if(!strcmp(name_str, "n_fvs_use_as_protos_set")) {
      *n_fvs_use_as_protos_set = atoi(val_str);
      setflags.n_fvs_use_as_protos_set = 1;
    }
    else if(!strcmp(name_str, "n_fvs_use_as_tuning_set")) {
      *n_fvs_use_as_tuning_set = atoi(val_str);
      setflags.n_fvs_use_as_tuning_set = 1;
    }
    else if(!strcmp(name_str, "outfile")) {
      strcpy(outfile, val_str);
      setflags.outfile = 1;
    }
    else if(!strcmp(name_str, "outfile_desc")) {
      strcpy(outfile_desc, val_str);
      setflags.outfile_desc = 1;
    }

    else
      fatalerr("optosf_read_parms (file optosf.c)",
        "illegal parm name", name_str);
  }
}

/********************************************************************/

/* Checks that every parm has been set. */

static void
optosf_check_parms_allset(void)
{
  if(!setflags.n_feats_use)
    fatalerr("optosf_check_parms_allset (file optosf.c)", "parm \
n_feats_use was never set", NULL);
  if(!setflags.osf_init)
    fatalerr("optosf_check_parms_allset (file optosf.c)", "parm \
osf_init was never set", NULL);
  if(!setflags.osf_initstep)
    fatalerr("optosf_check_parms_allset (file optosf.c)", "parm \
osf_initstep was never set", NULL);
  if(!setflags.osf_stepthr)
    fatalerr("optosf_check_parms_allset (file optosf.c)", "parm \
osf_stepthr was never set", NULL);
  if(!setflags.tablesize)
    fatalerr("optosf_check_parms_allset (file optosf.c)", "parm \
tablesize was never set", NULL);
  if(!setflags.verbose)
    fatalerr("optosf_check_parms_allset (file optosf.c)", "parm \
verbose was never set", NULL);
  if(!setflags.fvs_file)
    fatalerr("optosf_check_parms_allset (file optosf.c)", "parm \
fvs_file was never set", NULL);
  if(!setflags.classes_file)
    fatalerr("optosf_check_parms_allset (file optosf.c)", "parm \
classes_file was never set", NULL);
  if(!setflags.n_fvs_use_as_protos_set)
    fatalerr("optosf_check_parms_allset (file optosf.c)", "parm \
n_fvs_use_as_protos_set was never set", NULL);
  if(!setflags.n_fvs_use_as_tuning_set)
    fatalerr("optosf_check_parms_allset (file optosf.c)", "parm \
n_fvs_use_as_tuning_set was never set", NULL);
  if(!setflags.outfile)
    fatalerr("optosf_check_parms_allset (file optosf.c)", "parm \
outfile was never set", NULL);
  if(!setflags.outfile_desc)
    fatalerr("optosf_check_parms_allset (file optosf.c)", "parm \
outfile_desc was never set", NULL);
}

/********************************************************************/

static void
out_prog( char str[])
{
  fprintf(fp_out, "%s", str);
  fflush(fp_out);
  if(verbose_int)
    printf("%s", str);
}

/********************************************************************/
int main(int argc, char *argv[])
{
  char *prsfile, fvs_file[200], classes_file[200], outfile[200],
    outfile_desc[200], str[400], *datadir, *desc;
  unsigned char *classes;
  int n_feats_use, tablesize, n_fvs_use_as_protos_set,
    n_fvs_use_as_tuning_set;
  float osf_init, osf_initstep, osf_stepthr, osf, osf_step, osf_prev,
    acerror, acerror_prev, classerror, classerror_prev=1.0, *fvs;
  int n_cls;
  char **lcnptr;

  Usage("<prsfile>"); /* required user parameters file */
  prsfile = *++argv;

  /* Reads default optosf parameters file, then user parameters file,
  which overrides defaults. Checks that no parameter is left unset. */
  memset(&setflags, 0, sizeof(setflags));
  datadir = get_datadir();
  sprintf(str, "%s/parms/optosf.prs", datadir);
  optosf_read_parms(str, &n_feats_use, &osf_init, &osf_initstep,
    &osf_stepthr, &tablesize, &verbose_int, fvs_file, classes_file,
    &n_fvs_use_as_protos_set, &n_fvs_use_as_tuning_set, outfile,
    outfile_desc);
  optosf_read_parms(prsfile, &n_feats_use, &osf_init, &osf_initstep,
    &osf_stepthr, &tablesize, &verbose_int, fvs_file, classes_file,
    &n_fvs_use_as_protos_set, &n_fvs_use_as_tuning_set, outfile,
    outfile_desc);
  optosf_check_parms_allset();
  osf_prev=osf_init;

  if(n_fvs_use_as_tuning_set > n_fvs_use_as_protos_set) {
    sprintf(str,
        "n_fvs_use_as_tuning_set, %d, is > n_fvs_use_as_protos_set, %d",
	n_fvs_use_as_tuning_set, n_fvs_use_as_protos_set);
    fatalerr("optosf", str, NULL);
  }

  /* Read feature vectors and classes. */
  matrix_read_submatrix(tilde_filename(fvs_file, 0), 0,
    n_fvs_use_as_protos_set - 1, 0, n_feats_use - 1, &desc, &fvs);
  free(desc);
  classes_read_subvector_ind(tilde_filename(classes_file, 0), 0,
    n_fvs_use_as_protos_set - 1, &desc, &classes, &n_cls, &lcnptr);
  free(desc);
  free_dbl_char(lcnptr, n_cls);

  fp_out = fopen_ch(outfile, "wb");
  if(!strcmp(outfile_desc, "-"))
    fprintf(fp_out, "Optosf output file.  Parameters are: n_feats_use \
%d, osf_init %f, osf_initstep %f, osf_stepthr %f, fvs_file %s, \
classes_file %s, n_fvs_use_as_protos_set %d, \
n_fvs_use_as_tuning_set %d\n", n_feats_use, osf_init, osf_initstep,
      osf_stepthr, fvs_file, classes_file, n_fvs_use_as_protos_set,
      n_fvs_use_as_tuning_set);
  else
    fprintf(fp_out, "%s\n", outfile_desc);
  fflush(fp_out);

  /* Optimize osf by a very simple method.  Start off taking large
  steps, and if the error fails to decrease then reverse direction
  and halve the step size.  Stop when the step size becomes small.
  Store previously computed (osf,error) pairs for lookup, to prevent
  wasting cycles computing the error function more than once for the
  same input value. */
  optosf_pnn(n_feats_use, n_fvs_use_as_protos_set,
    n_fvs_use_as_tuning_set, fvs, classes, n_cls, osf_init, &acerror_prev,
    &classerror);
  sprintf(str, "osf: %f; activ. error: %f; classif. error: %f\n",
    osf_init, acerror_prev, classerror);
  out_prog(str);
  for(osf = osf_init + (osf_step = osf_initstep); ; osf += osf_step) {
    optosf_pnn(n_feats_use, n_fvs_use_as_protos_set,
      n_fvs_use_as_tuning_set, fvs, classes, n_cls, osf, &acerror,
      &classerror);
    sprintf(str, "osf: %f; activ. error: %f; classif. error: %f\n",
      osf, acerror, classerror);
    out_prog(str);
    if(acerror >= acerror_prev) {
      if(fabs((double)osf_step) <= osf_stepthr)
	break;
      osf_step /= -2;
    }
    osf_prev = osf;
    acerror_prev = acerror;
    classerror_prev = classerror;
  }

  /* Optimal osf. */
  sprintf(str, "Optimization finished; producing:\n  osf: %f; \
activ. error: %f; classif. error: %f\n", osf_prev, acerror_prev,
    classerror_prev);
  out_prog(str);
  return 0;
}

