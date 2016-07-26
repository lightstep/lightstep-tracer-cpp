#!/bin/sh

set -e

if [ `uname` = "Darwin" ]; then
    # TODO On Mac OS X building against "keg-only" libraries (e.g., the
    # Thrift you built depends on a keg-only openssl) you'll have to
    # ammend your pkg-config path:

    export PKG_CONFIG_PATH=$(brew --prefix)/opt/openssl/lib/pkgconfig
    export OPENSSL_ROOT_DIR=`pkg-config --variable=prefix openssl`
fi

./bootstrap.sh 1> /dev/null 2> /dev/null
./configure
make
