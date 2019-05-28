#!/bin/bash

set -e

[ -z "${SRC_DIR}" ] && export SRC_DIR="`pwd`"
[ -z "${BUILD_DIR}" ] && export BUILD_DIR=/build
mkdir -p "${BUILD_DIR}"

BAZEL_OPTIONS=""
BAZEL_TEST_OPTIONS="$BAZEL_OPTIONS --test_output=errors"

function copy_benchmark_results() {
  mkdir -p "${BENCHMARK_DST_DIR}"
  cd "${BENCHMARK_SRC_DIR}"
  find . -name '*_result.txt' -exec bash -c \
    'echo "$@" && mkdir -p "${BENCHMARK_DST_DIR}"/$(dirname "$@") && \
     cp "$@" "${BENCHMARK_DST_DIR}"/"$@"' _ {} \;
}

function setup_clang_toolchain() {
  export PATH=/usr/lib/llvm-6.0/bin:$PATH
  export CC=clang
  export CXX=clang++
  export ASAN_SYMBOLIZER_PATH=/usr/lib/llvm-6.0/bin/llvm-symbolizer
  export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/lib/llvm-6.0/lib/clang/6.0.1/lib/linux/
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
        -DWIH_CARES=ON \
        "${SRC_DIR}"
  make
  make test
  exit 0
elif [[ "$1" == "clang_tidy" ]]; then
  setup_clang_toolchain
  bazel build \
        $BAZEL_OPTIONS \
        //src/... //test/... //benchmark/... //include/...
  ./ci/gen_compilation_database.sh
  ./ci/fix_compilation_database.py
  ./ci/run_clang_tidy.sh |& tee /clang-tidy-result.txt
  grep ": error:" /clang-tidy-result.txt | cat > /clang-tidy-errors.txt
  num_errors=`wc -l /clang-tidy-errors.txt | awk '{print $1}'`
  if [[ $num_errors -ne 0 ]]; then
    exit 1
  fi
  exit 0
elif [[ "$1" == "bazel.test" ]]; then
  bazel build -c dbg $BAZEL_OPTIONS //...
  bazel test -c dbg $BAZEL_TEST_OPTIONS //...
  exit 0
elif [[ "$1" == "bazel.asan" ]]; then
  setup_clang_toolchain
  bazel test -c dbg \
        $BAZEL_TEST_OPTIONS \
        --config=asan \
        -- //... \
           -//test/bridge/... \
           -//test/tracer:dynamic_load_test
  exit 0
elif [[ "$1" == "bazel.tsan" ]]; then
  setup_clang_toolchain
  bazel test -c dbg \
        $BAZEL_TEST_OPTIONS \
        --config=tsan \
        -- //... \
           -//test/bridge/... \
           -//test/tracer:dynamic_load_test
  exit 0
elif [[ "$1" == "bazel.benchmark" ]]; then
  export BENCHMARK_SRC_DIR=bazel-genfiles/benchmark
  export BENCHMARK_DST_DIR=/benchmark
  bazel build -c opt \
        $BAZEL_OPTIONS \
        //benchmark/...
  copy_benchmark_results
  exit 0
elif [[ "$1" == "bazel.coverage" ]]; then
  mkdir -p /coverage
  rm -rf /coverage/*
  bazel coverage \
    $BAZEL_OPTIONS \
    --instrument_test_targets \
    --experimental_cc_coverage \
    --combined_report=lcov \
    --instrumentation_filter="-3rd_party,-benchmark,-test,-bridge" \
    --coverage_report_generator=@bazel_tools//tools/test/CoverageOutputGenerator/java/com/google/devtools/coverageoutputgenerator:Main \
    -- \
    //... \
    -//test/bridge/...
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
