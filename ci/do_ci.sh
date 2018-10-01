#!/bin/bash

set -e

[ -z "${SRC_DIR}" ] && export SRC_DIR="`pwd`"
[ -z "${BUILD_DIR}" ] && export BUILD_DIR=/build
mkdir -p "${BUILD_DIR}"

function run_upload_benchmark {
  for recorder in rpc stream; do
    for max_buffered_spans in 2000; do
      for threads in 1 2 4 8; do
        for max_spans_per_second in 5000 10000 20000 50000 100000 200000 500000 1000000; do
          OUTPUT_FILE=/benchmark-results/upload_${recorder}.out
          echo "**********************************" >> $OUTPUT_FILE
          echo "./benchmark/upload/upload_benchmark \
              --recorder_type ${recorder} \
              --max_buffered_spans ${max_buffered_spans} \
              --num_threads ${threads} \
              --num_spans 500000 \
              --max_spans_per_second ${max_spans_per_second}"
          ./benchmark/upload/upload_benchmark \
              --recorder_type ${recorder} \
              --max_buffered_spans ${max_buffered_spans} \
              --num_threads ${threads} \
              --num_spans 500000 \
              --max_spans_per_second ${max_spans_per_second} >> $OUTPUT_FILE
        done
      done
    done
  done
}

if [[ "$1" == "cmake.debug" ]]; then
  cd "${BUILD_DIR}"
  cmake -DBUILD_BENCHMARKING=ON \
        -DCMAKE_BUILD_TYPE=Debug  \
        -DCMAKE_CXX_FLAGS="-Werror" \
        "${SRC_DIR}"
  make
  make test
  exit 0
elif [[ "$1" == "cmake.no_grpc" ]]; then
  cd "${BUILD_DIR}"
  cmake -DCMAKE_BUILD_TYPE=Debug  \
        -DCMAKE_CXX_FLAGS="-Werror" \
        -DWITH_GRPC=OFF \
        "${SRC_DIR}"
  make
  make test
  exit 0
elif [[ "$1" == "cmake.asan" ]]; then
  cd "${BUILD_DIR}"
  cmake -DBUILD_BENCHMARKING=ON \
        -DCMAKE_BUILD_TYPE=Debug  \
        -DCMAKE_CXX_FLAGS="-Werror -fno-omit-frame-pointer -fsanitize=address"  \
        -DCMAKE_SHARED_LINKER_FLAGS="-fno-omit-frame-pointer -fsanitize=address" \
        -DCMAKE_EXE_LINKER_FLAGS="-fno-omit-frame-pointer -fsanitize=address" \
        "${SRC_DIR}"
  make
  make test
  exit 0
elif [[ "$1" == "cmake.tsan" ]]; then
  cd "${BUILD_DIR}"
# Testing with dynamic load seems to have some issues with TSAN so turn off
# dynamic loading in this test for now.
  TSAN_FLAGS="-fno-omit-frame-pointer -fsanitize=thread \
    -fsanitize-blacklist=${SRC_DIR}/ci/tsan-blacklist.txt"
  cmake -DBUILD_BENCHMARKING=ON \
        -DCMAKE_BUILD_TYPE=Debug  \
        -DCMAKE_CXX_FLAGS="-Werror $TSAN_FLAGS"  \
        -DCMAKE_SHARED_LINKER_FLAGS="$TSAN_FLAGS" \
        -DCMAKE_EXE_LINKER_FLAGS="$TSAN_FLAGS" \
        -DWITH_DYNAMIC_LOAD=OFF \
        "${SRC_DIR}"
  make VERBOSE=1
  make test
  exit 0
elif [[ "$1" == "coverage" ]]; then
  cd "${BUILD_DIR}"
  cmake -DBUILD_BENCHMARKING=ON \
        -DCMAKE_BUILD_TYPE=Debug  \
    -DCMAKE_CXX_FLAGS="-fprofile-arcs -ftest-coverage -fPIC -O0" \
    "$SRC_DIR"
  make VERBOSE=1
  make test
  cd CMakeFiles/lightstep_tracer.dir/src
  gcovr -r "$SRC_DIR" . --html --html-details -o coverage.html
  mkdir /coverage
  cp *.html /coverage/
  exit 0
elif [[ "$1" == "benchmark" ]]; then
  cd "${BUILD_DIR}"
  cmake -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_BENCHMARKING=ON \
        -DBUILD_TESTING=OFF \
        "${SRC_DIR}"
  make VERBOSE=1
  mkdir /benchmark-results
  ./benchmark/span_operations_benchmark --benchmark_color=false > /benchmark-results/span_operations_benchmark.out 2>&1
  run_upload_benchmark
  exit 0
elif [[ "$1" == "cmake.clang-tidy" ]]; then
  cd "${BUILD_DIR}"
  cmake -DBUILD_BENCHMARKING=ON \
        -DCMAKE_BUILD_TYPE=Debug  \
        -DCMAKE_C_COMPILER=/usr/bin/clang-6.0 \
        -DCMAKE_CXX_COMPILER=/usr/bin/clang++-6.0 \
        -DENABLE_LINTING=ON \
        -DCMAKE_CXX_FLAGS="-Werror" \
        "${SRC_DIR}"
  make
  make test
  exit 0
elif [[ "$1" == "plugin" ]]; then
  cd "${BUILD_DIR}"
  "${SRC_DIR}"/ci/build_plugin.sh
  exit 0
elif [[ "$1" == "bazel.build" ]]; then
  bazel build //...
  exit 0
elif [[ "$1" == "release" ]]; then
  "${SRC_DIR}"/ci/build_plugin.sh
  "${SRC_DIR}"/ci/release.sh
  exit 0
fi
