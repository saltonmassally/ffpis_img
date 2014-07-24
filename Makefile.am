## Process this file with automake to produce Makefile.in
#  Copyright (C) 2003,2004 Stephen Pollei

AUTOMAKE_OPTIONS= gnu 1.6
#SUBDIRS = intl src po doc
SUBDIRS = src debian
EXTRA_DIST= HACKING TODO LGPL-2.1.txt MPL-1.1.txt
pkgconfigdir= @pkgconfigdir@
pkgconfig_DATA = ffpis_img.pc

NUKE_CLEAN_FILES = $(CONFIG_CLEAN_FILES) $(am__CONFIG_DISTCLEAN_FILES) \
	config.h stamp-h1 *.lo configure install-sh missing mkinstalldirs \
	Makefile.in aclocal.m4 config.h.in stamp-h.in config.guess \
	config.sub ltconfig ltmain.sh depcomp src/Makefile.in debian/Makefile.in

nuke-clean :  clean clean-am maintainer-clean distclean-am 
	cd $(top_srcdir) && $(RM) $(NUKE_CLEAN_FILES)
	cd $(top_srcdir)/aux_dir && $(RM) $(NUKE_CLEAN_FILES)
	cd autom4te-*.cache/ && $(RM) output.* requests traces.*
	rmdir autom4te-*.cache/ aux_dir
	$(RM) $(NUKE_CLEAN_FILES)
