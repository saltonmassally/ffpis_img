#! /bin/sh
mkdir aux_dir
aclocal
autoheader
libtoolize
automake -a
autoconf
