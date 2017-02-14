#!/bin/sh

# In case this source is being cross-compiled.
make maintainer-clean 2> /dev/null
rm -rf autom4te.cache 2> /dev/null

LIBTOOLIZE=libtoolize
if [ `uname` = 'Darwin' ]; then
  LIBTOOLIZE=glibtoolize
fi

${LIBTOOLIZE} --copy --automake

aclocal -I m4
autoheader
autoconf
automake --copy --add-missing
