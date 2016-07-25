#!/bin/sh

set -e

./bootstrap.sh 1> /dev/null 2> /dev/null
./configure
make
