#!/bin/bash

set -e

BENCHMARK_VERSION=v1.4.1
git clone -b $BENCHMARK_VERSION https://github.com/google/benchmark.git google-benchmark
cd google-benchmark
mkdir build
cd build
cmake .. -DBENCHMARK_ENABLE_GTEST_TESTS=OFF -DCMAKE_BUILD_TYPE=RELEASE
make
make install
