/************************************************************************

      PACKAGE:  PCASYS TOOLS

      FILE:     OPTRWSGW.C

      AUTHORS:  Craig Watson
                cwatson@nist.gov
                G. T. Candela
      DATE:     08/01/1995

#cat: optrwsgw - Optimizes the regional weights that are then applied
#cat:            to the eigen vectors. Used to run optimization
#cat:            on several processors at onetime.

The optrws (optimize regional weights) command can run several
simultaneous instances of this program, for faster estimation of the
gradient if there are several processors available.  Each instance
computes a segment of the estimated partials.

*************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ffpis/util/little.h>
#include <ffpis/util/optrws_r.h>
#include <ffpis/util/memalloc.h>
#include <ffpis/util/util.h>
#include <ffpis/util/ioutil.h>
#include <ffpis/util/datafile.h>

int main(int argc, char *argv[])
{
  char *desc, *klfvs_file, *classes_file, *eigvecs_file,
    *rws_bspt_file, *temp_outfile;
  unsigned char *classes;
  int n_feats_use, n_klfvs_use, seg_start, seg_end, seg_nbytes, i, fd,
    rws_bspt_file_full_nbytes;
  float *klfvs, *eigvecs, *rws_bspt, grad_est_stepsize,
    *seg_acerrors_stepped, *rws, *p;
  int rw, rh, rwsz;
  int n_feats, evt_sz, n_cls;
  char **lcnptr;

  if(argc != 12)
    fatalerr("This command should be run only by optrws", (char *)NULL,
             (char *)NULL);
  n_feats_use = atoi(*++argv);
  n_klfvs_use = atoi(*++argv);
  klfvs_file = *++argv;
  classes_file = *++argv;
  eigvecs_file = *++argv;
  rws_bspt_file = *++argv;
  rws_bspt_file_full_nbytes = atoi(*++argv);
  seg_start = atoi(*++argv);
  seg_end = atoi(*++argv);
  grad_est_stepsize = atof(*++argv);
  temp_outfile = *++argv;

  /* Read K-L feature vectors and their classes, and eigenvectors. */
  matrix_read_submatrix(klfvs_file, 0, n_klfvs_use - 1, 0,
    n_feats_use - 1, &desc, &klfvs);
  free(desc);

  classes_read_subvector_ind(classes_file, 0, n_klfvs_use - 1, &desc,
    &classes, &n_cls, &lcnptr);
  free(desc);
  free_dbl_char(lcnptr, n_cls);
  matrix_read_dims(eigvecs_file, &n_feats, &evt_sz);
  matrix_read_submatrix(eigvecs_file, 0, n_feats_use - 1, 0, evt_sz-1,
    &desc, &eigvecs);
  free(desc);

  /* Read the current basepoint, as written by optrws, which is
  running as the parent process of this process.  But do not attempt
  to read it until it is of the expected size. (There may be a delay
  between the production of this file, and its appearance in full on
  a remote system.) */
  while(filesize(rws_bspt_file) != rws_bspt_file_full_nbytes)
    sleep(1);
  matrix_read(rws_bspt_file, &desc, &rh, &rw, &rws_bspt);
  free(desc);
  rwsz = rw*rh;

  /* For each of the segment of the coordinates assigned to this
  instance of optrwsgw, take a step from the basepoint along that
  coordinate and compute activation error rate. */
  seg_acerrors_stepped = (float *)malloc_ch(seg_nbytes = (seg_end -
    seg_start) * sizeof(float));
  malloc_flt(&rws, rwsz, "optrwsgw rws");
  for(i = seg_start, p = seg_acerrors_stepped; i < seg_end; i++) {
    memcpy(rws, rws_bspt, rwsz * sizeof(float));
    rws[i] += grad_est_stepsize;
    *p++ = rws_to_acerror(rws, rw, rh, eigvecs, evt_sz, n_feats_use, n_klfvs_use,
      klfvs, classes, n_cls);
  }
  free(rws);

  /* Write error values as a temporary file, which optrws will read,
  along with the files from the other optrwsgw instances. */
  fd = creat_ch(temp_outfile);
  write(fd, seg_acerrors_stepped, seg_nbytes);
  close(fd);
  return 0;
}
