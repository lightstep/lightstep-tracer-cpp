#!/bin/bash

set -e

[ -z "${SRC_DIR}" ] && export SRC_DIR="`pwd`"
[ -z "${BUILD_DIR}" ] && export BUILD_DIR=/build
mkdir -p "${BUILD_DIR}"

BAZEL_OPTIONS=""
BAZEL_TEST_OPTIONS="$BAZEL_OPTIONS --test_output=errors"
BAZEL_PLUGIN_OPTIONS="$BAZEL_OPTIONS \
              -c opt \
              --copt=-march=x86-64 \
              --config=portable_glibc \
              --config=static_libcpp \
  "

function copy_benchmark_results() {
  mkdir -p "${BENCHMARK_DST_DIR}"
  pushd "${BENCHMARK_SRC_DIR}"
  find . -name '*_result.txt' -exec bash -c \
    'echo "$@" && mkdir -p "${BENCHMARK_DST_DIR}"/$(dirname "$@") && \
     cp "$@" "${BENCHMARK_DST_DIR}"/"$@"' _ {} \;
  popd
}

function copy_benchmark_profiles() {
  mkdir -p "${BENCHMARK_DST_DIR}"
  pushd "${BENCHMARK_SRC_DIR}"
  find . -name '*_profile.tgz' -exec bash -c \
    'echo "$@" && mkdir -p "${BENCHMARK_DST_DIR}"/$(dirname "$@") && \
     tar zxf "$@" -C "${BENCHMARK_DST_DIR}"/$(dirname "$@")' _ {} \;
  popd
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
        -- //src/... //bridge/... //test/... //include/... //benchmark/...
  ./ci/gen_compilation_database.sh
  ./ci/fix_compilation_database.py
  ./ci/run_clang_tidy.sh |& tee /clang-tidy-result.txt
  grep ": error:" /clang-tidy-result.txt | cat > /clang-tidy-errors.txt
  num_errors=`wc -l /clang-tidy-errors.txt | awk '{print $1}'`
  if [[ $num_errors -ne 0 ]]; then
    exit 1
  fi
  exit 0
elif [[ "$1" == "bazel.build" ]]; then
  bazel build -c dbg $BAZEL_OPTIONS -- //... -//benchmark/...
  exit 0
elif [[ "$1" == "bazel.test" ]]; then
  bazel build -c dbg $BAZEL_OPTIONS -- //... -//benchmark/...
  bazel test -c dbg $BAZEL_TEST_OPTIONS //...
  exit 0
elif [[ "$1" == "bazel.unsigned_char.test" ]]; then
  bazel build -c dbg --copt -funsigned-char $BAZEL_OPTIONS -- //... -//benchmark/...
  bazel test -c dbg --copt -funsigned-char $BAZEL_TEST_OPTIONS //...
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
           -//test/tracer:dynamic_load_test \
           -//test/recorder/stream_recorder:stream_recorder_fork_test # This test hangs forever when run with tsan
  exit 0
elif [[ "$1" == "bazel.benchmark" ]]; then
  export BENCHMARK_SRC_DIR=bazel-bin/benchmark
  export BENCHMARK_DST_DIR=/benchmark
  bazel build -c opt \
        $BAZEL_OPTIONS \
        //benchmark/...
  copy_benchmark_results
  copy_benchmark_profiles
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
  bazel build $BAZEL_PLUGIN_OPTIONS \
    //:liblightstep_tracer_plugin.so \
    //benchmark/tracer_upload_bench:tracer_upload_bench \
    //test/mock_satellite:mock_satellite \
    //test/mock_satellite:mock_satellite_query \
    //test/tracer:span_probe \
    //bridge/python:wheel.tgz
  mkdir -p /plugin
  cp bazel-bin/liblightstep_tracer_plugin.so /plugin
  cp bazel-bin/benchmark/tracer_upload_bench/tracer_upload_bench /plugin
  cp bazel-bin/test/mock_satellite/mock_satellite /plugin
  cp bazel-bin/test/mock_satellite/mock_satellite_query /plugin
  cp bazel-bin/test/tracer/span_probe /plugin
  cp bazel-bin/bridge/python/wheel.tgz /
  cd /
  tar zxf wheel.tgz
  cp wheel/* plugin/
  exit 0
elif [[ "$1" == "plugin.test" ]]; then
  MOCK_SATELLITE_PORT=5000
  /plugin/mock_satellite $MOCK_SATELLITE_PORT &
  MOCK_SATELLITE_PID=$!
  /plugin/span_probe /plugin/liblightstep_tracer_plugin.so 127.0.0.1 $MOCK_SATELLITE_PORT
  NUM_SPANS=`/plugin/mock_satellite_query --port $MOCK_SATELLITE_PORT num_spans`
  kill $MOCK_SATELLITE_PID
  [[ "$NUM_SPANS" -eq "1" ]] || exit 1
  exit 0
elif [[ "$1" == "python.wheel.test" ]]; then
  # Python 3
  python3 -m pip install /plugin/*cp3*.whl
  MOCK_SATELLITE_PORT=5000
  /plugin/mock_satellite $MOCK_SATELLITE_PORT &
  MOCK_SATELLITE_PID=$!
  python3 test/bridge/python/span_probe.py 127.0.0.1 $MOCK_SATELLITE_PORT
  NUM_SPANS=`/plugin/mock_satellite_query --port $MOCK_SATELLITE_PORT num_spans`
  kill $MOCK_SATELLITE_PID
  [[ "$NUM_SPANS" -eq "1" ]] || exit 1

  # Python 2.7
  python -m pip install /plugin/*cp27mu*.whl
  MOCK_SATELLITE_PORT=5000
  /plugin/mock_satellite $MOCK_SATELLITE_PORT &
  MOCK_SATELLITE_PID=$!
  python test/bridge/python/span_probe.py 127.0.0.1 $MOCK_SATELLITE_PORT
  NUM_SPANS=`/plugin/mock_satellite_query --port $MOCK_SATELLITE_PORT num_spans`
  kill $MOCK_SATELLITE_PID
  [[ "$NUM_SPANS" -eq "1" ]] || exit 1
  exit 0
elif [[ "$1" == "release" ]]; then
  "${SRC_DIR}"/ci/release.sh
  exit 0
fi
