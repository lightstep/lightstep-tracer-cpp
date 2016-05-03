#!/bin/sh

# TODO Make this portable. Written for OS X.
glibtoolize --copy --automake
aclocal -I m4
autoheader
autoconf
automake --copy --add-missing
