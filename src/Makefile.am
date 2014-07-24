## Process this file with automake to produce Makefile.in
#  Copyright (C) 2003 Stephen Pollei

AUTOMAKE_OPTIONS= gnu 1.6
gcc_warn_flags = -Wall -W -Wstrict-prototypes -Wsign-compare -Wcast-qual -Wbad-function-cast -Wmissing-prototypes
CFLAGS= @CFLAGS@  @FFPIS_UTIL_CFLAGS@ @JPEGB_CFLAGS@ $(gcc_warn_flags)
LDFLAGS = @FFPIS_UTIL_LIBS@
ffpis_includedir = @ffpis_includedir@
ffpis_img_includedir = @ffpis_img_includedir@
ffpis_libdir = @ffpis_libdir@
# @FFPIS_UTIL_LIBS@
# @FFPIS_UTIL_CFLAGS@
# @FFPIS_MATH_LIBS@
# @FFPIS_MATH_CFLAGS@
# @FFPIS_IMG_LIBS@
# @FFPIS_IMG_CFLAGS@

ffpis_lib_LTLIBRARIES = libffpis_img.la
bin_PROGRAMS = asc2bin bin2asc cjpegl chgdesc cmbmcs cwsq djpegl djpeglsd \
	dwsq14 dwsq fixwts intr2not kltran lintran meancov mktran \
	mlpfeats not2intr optosf oas2pics optrws optrwsgw rdwsqcom \
	rgb2ycc rwpics sd_rfmt stackms wrwsqcom ycc2rgb dpyimage
# EXTRA_PROGRAMS = 
cjpegl_LDADD = libffpis_img.la
cwsq_LDADD = libffpis_img.la
djpegl_LDADD = libffpis_img.la
djpeglsd_LDADD = libffpis_img.la
dwsq14_LDADD = libffpis_img.la
dwsq_LDADD = libffpis_img.la
intr2not_LDADD = libffpis_img.la
oas2pics_LDADD = libffpis_img.la
not2intr_LDADD = libffpis_img.la
rdwsqcom_LDADD = libffpis_img.la
rgb2ycc_LDADD = libffpis_img.la
rwpics_LDADD = libffpis_img.la
sd_rfmt_LDADD = libffpis_img.la
wrwsqcom_LDADD = libffpis_img.la
ycc2rgb_LDADD = libffpis_img.la
dpyimage_LDADD = libffpis_img.la
dpyimage_LDFLAGS = @LDFLAGS@ @X_LIBS@ @X_PRE_LIBS@ -lX11
dpyimage_SOURCES = dpyimage.c dpyio.c dpymain.c dpynorm.c \
	dpypipe.c dpytmp.c dpyx.c tally.c

FETSRC = allocfet.c delfet.c extrfet.c freefet.c lkupfet.c \
	nistcom.c printfet.c readfet.c strfet.c updatfet.c writefet.c

IHEADSRC = getnset.c getcomp.c nullihdr.c parsihdr.c prntihdr.c \
	readihdr.c valdcomp.c writihdr.c

IMAGESRC = binfill.c bincopy.c binpad.c copy.c bitmasks.c findblob.c \
	grp4comp.c grp4deco.c imageops.c img_io.c imgdecod.c \
	imgutil.c imgtype.c intrlv.c masks.c parsargs.c \
	rgb_ycc.c rl.c sunrast.c
# readihdr.c and writeihdr.c were dups with ihead

JPEGLSRC = decoder.c encoder.c huff.c huftable.c \
	imgdat.c ppi.c sd4util.c tableio.c util.c

JPEGBSRC = jpegb_decoder.c jpegb_encoder.c jpegb_marker.c \
	jpegb_membuf.c jpegb_ppi.c

WSQSRC = wsq_decoder.c wsq_encoder.c wsq_globals.c wsq_huff.c wsq_ppi.c \
	sd14util.c wsq_tableio.c wsq_tree.c wsq_util.c



libffpis_img_la_SOURCES = $(FETSRC) $(IHEADSRC) \
	$(IMAGESRC) $(JPEGLSRC) $(JPEGBSRC) $(WSQSRC)
libffpis_img_la_LDFLAGS = @LIBS@ @JPEGB_LIBS@


noinst_HEADERS = dpyimage.h dpyx.h jerror.h jmorecfg.h

ffpis_img_include_HEADERS = binops.h bitmasks.h bits.h computil.h copy.h \
	dataio.h defs.h fet.h findblob.h getnset.h grp4comp.h grp4deco.h \
	ihead.h imgdec.h imgdecod.h img_io.h imgtype.h imgutil.h intrlv.h \
	invbyte.h jpegb.h jpegl.h \
	jpeglsd4.h masks.h memalloc.h nistcom.h parsargs.h rgb_ycc.h \
	sunrast.h swap.h wsq.h

