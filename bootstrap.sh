#!/bin/sh

# TODO Make this portable. Written for OS X.
LIBTOOLIZE=libtoolize
if [ `uname` = 'Darwin' ]; then
  LIBTOOLIZE=glibtoolize
fi
${LIBTOOLIZE} --copy --automake
aclocal -I m4
autoheader
autoconf
automake --copy --add-missing
