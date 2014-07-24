#ifndef _DPY_H
#define _DPY_H

#include <sys/param.h>

#define DPY_NORM		0
#define DPY_PIPE		1
#define DPY_TMP			2

#define DEF_BORDER_WIDTH	4
#define DEF_SLEEPTIME		2
#define DEF_TMPDIR		"/tmp"

#define OUTFILE_DIRMODE		0700
#define OUTFILE_DIRFMT		"%s/dpy_%d"
#define OUTFILE_FMT		"%s/dpy_%d/%d"
#define OUTFILE_EXT		".ctrl"

struct header_t {
	char filename[MAXPATHLEN];
	u_int iw, ih, depth, whitepix;
	int align;
};

#define HEADERSIZE		(sizeof(struct header_t))

#endif /* !_DPY_H */
