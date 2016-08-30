#!/bin/sh

set -e

./bootstrap.sh 1> /dev/null 2> /dev/null
# TODO enable gRPC
./configure --disable-grpc
make
