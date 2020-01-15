load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_cc_library",
    "lightstep_package",
)

lightstep_package()

lightstep_cc_library(
    name = "report_request_lib",
    private_hdrs = [
        "report_request.h",
    ],
    srcs = [
        "report_request.cpp",
    ],
    deps = [
        "//include/lightstep:buffer_chain_interface",
    ],
)

lightstep_cc_library(
    name = "report_request_header_lib",
    private_hdrs = [
        "report_request_header.h",
    ],
    srcs = [
        "report_request_header.cpp",
    ],
    deps = [
        "//include/lightstep:tracer_interface",
        "//lightstep-tracer-common:collector_proto_cc",
        "//src/common:protobuf_lib",
        "//src/common:utility_lib",
    ],
)
