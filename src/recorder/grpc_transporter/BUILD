load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_cc_library",
    "lightstep_package",
)

lightstep_package()

lightstep_cc_library(
    name = "grpc_transporter_lib",
    srcs = [
        "grpc_transporter.cpp",
    ],
    deps = [
        "//src/recorder:grpc_transporter_interface",
        "//src/common:logger_lib",
        "//include/lightstep:tracer_interface",
        "//lightstep-tracer-common:collector_proto_grpc",
    ],
)
