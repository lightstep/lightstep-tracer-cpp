#!/bin/bash

set -e

[ -z "${SRC_DIR}" ] && export SRC_DIR="`pwd`"
[ -z "${BUILD_DIR}" ] && export BUILD_DIR=/build
mkdir -p "${BUILD_DIR}"

BAZEL_OPTIONS="--jobs 1"
BAZEL_TEST_OPTIONS="$BAZEL_OPTIONS --test_output=errors"

function copy_benchmark_results() {
  mkdir -p "${BENCHMARK_DST_DIR}"
  cd "${BENCHMARK_SRC_DIR}"
  find . -name '*_result.txt' -exec bash -c \
    'echo "$@" && mkdir -p "${BENCHMARK_DST_DIR}"/$(dirname "$@") && \
     cp "$@" "${BENCHMARK_DST_DIR}"/"$@"' _ {} \;
}

if [[ "$1" == "cmake.minimal" ]]; then
  cd "${BUILD_DIR}"
  cmake -DCMAKE_BUILD_TYPE=Debug  \
        -DCMAKE_CXX_FLAGS="-Werror" \
        -DWITH_GRPC=OFF \
        -DWITH_LIBEVENT=OFF \
        "${SRC_DIR}"
  make
  exit 0
elif [[ "$1" == "cmake.full" ]]; then
  cd "${BUILD_DIR}"
  cmake -DCMAKE_BUILD_TYPE=Debug  \
        -DCMAKE_CXX_FLAGS="-Werror" \
        -DWITH_GRPC=ON \
        -DWITH_LIBEVENT=ON \
        "${SRC_DIR}"
  make
  make test
  exit 0
elif [[ "$1" == "bazel.asan" ]]; then
  bazel build -c dbg \
        $BAZEL_OPTIONS \
        --copt=-fsanitize=address \
        --linkopt=-fsanitize=address \
        //...
  bazel test -c dbg \
        $BAZEL_TEST_OPTIONS \
        --copt=-fsanitize=address \
        --linkopt=-fsanitize=address \
        //...
  exit 0
elif [[ "$1" == "bazel.tsan" ]]; then
  bazel build -c dbg \
        $BAZEL_OPTIONS \
        --copt=-fsanitize=thread \
        --linkopt=-fsanitize=thread \
        //...
  bazel test -c dbg \
        $BAZEL_TEST_OPTIONS \
        --copt=-fsanitize=thread \
        --linkopt=-fsanitize=thread \
        //...
  exit 0
elif [[ "$1" == "bazel.benchmark" ]]; then
  export BENCHMARK_SRC_DIR=bazel-genfiles/benchmark
  export BENCHMARK_DST_DIR=/benchmark
  bazel build -c opt \
        $BAZEL_OPTIONS \
        //benchmark/...
  copy_benchmark_results
  exit 0
elif [[ "$1" == "coverage" ]]; then
  mkdir -p /coverage
  rm -rf /coverage/*
  bazel coverage \
    $BAZEL_OPTIONS \
    --instrument_test_targets \
    --experimental_cc_coverage \
    --combined_report=lcov \
    --instrumentation_filter="-3rd_party,-benchmark,-test" \
    --coverage_report_generator=@bazel_tools//tools/test/CoverageOutputGenerator/java/com/google/devtools/coverageoutputgenerator:Main \
    //...
  genhtml bazel-out/_coverage/_coverage_report.dat \
          --output-directory /coverage
  tar czf /coverage.tgz /coverage
  exit 0
elif [[ "$1" == "plugin" ]]; then
  cd "${BUILD_DIR}"
  "${SRC_DIR}"/ci/build_plugin.sh
  exit 0
elif [[ "$1" == "release" ]]; then
  "${SRC_DIR}"/ci/build_plugin.sh
  "${SRC_DIR}"/ci/release.sh
  exit 0
fi
