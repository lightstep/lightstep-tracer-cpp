load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_cc_library",
    "lightstep_python_wheel",
    "lightstep_package",
)

lightstep_package()

lightstep_cc_library(
    name = "python_tracer_lib",
    deps = [
      "//src/tracer:lightstep_tracer_factory_lib",
      "//src/tracer:tracer_lib",
      "//src/tracer:binary_carrier_lib",
      "//src/tracer:counting_metrics_observer_lib",
      "//src/tracer:no_default_ssl_roots_pem_lib",
      "//src/recorder:transporter_lib",
      "//src/recorder:no_grpc_transporter_lib",
      "//src/recorder/stream_recorder:stream_recorder_lib",
      "//src/network/ares_dns_resolver:ares_dns_resolver_lib",
    ],
)

filegroup(
    name = "wheel_files",
    srcs = glob(["wheel/*"]),
)

lightstep_python_wheel(
  python_tag = "cp32",
  abi_tag = "abi3",
  binary = "//bridge/python/binary/py3:lightstep_streaming.so",
)

lightstep_python_wheel(
  python_tag = "cp27",
  abi_tag = "cp27m",
  binary = "//bridge/python/binary/py27m:lightstep_streaming.so",
)

lightstep_python_wheel(
  python_tag = "cp27",
  abi_tag = "cp27mu",
  binary = "//bridge/python/binary/py27mu:lightstep_streaming.so",
)

genrule(
    name = "generate_wheels",
    srcs = [
        ":wheel-cp32-abi3.tgz",
        ":wheel-cp27-cp27m.tgz",
        ":wheel-cp27-cp27mu.tgz",
    ],
    outs = [
        "wheel.tgz",
    ],
    cmd = """
    WORK_DIR=`mktemp -d`
    cp $(location :wheel-cp32-abi3.tgz) $$WORK_DIR
    cp $(location :wheel-cp27-cp27m.tgz) $$WORK_DIR
    cp $(location :wheel-cp27-cp27mu.tgz) $$WORK_DIR
    pushd $$WORK_DIR    
    for file in *.tgz; do
      tar zxf $$file
    done
    tar czf wheel.tgz wheel
    popd
    cp $$WORK_DIR/wheel.tgz $${PWD}/$(location :wheel.tgz)
    rm -rf $$WORK_DIR
    """,
)
