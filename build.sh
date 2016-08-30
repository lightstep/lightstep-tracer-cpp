#!/bin/sh

set -e

if [ `uname` = "Darwin" ]; then
    # TODO To use pkg-config on Mac OS X building against "keg-only" libraries:

    export PKG_CONFIG_PATH=$(brew --prefix)/opt/grpc/lib/pkgconfig:$PKG_CONFIG_PATH
    export PKG_CONFIG_PATH=$(brew --prefix)/opt/protobuf/lib/pkgconfig:$PKG_CONFIG_PATH
fi

./bootstrap.sh 1> /dev/null 2> /dev/null
# TODO enable gRPC by default, when it's functional
./configure --disable-grpc
make
