#!/bin/bash

set -e

BENCHMARK_VERSION=v1.4.1
git clone -b $BENCHMARK_VERSION https://github.com/google/benchmark.git google-benchmark
cd google google-benchmark
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=RELEASE
make
make install
