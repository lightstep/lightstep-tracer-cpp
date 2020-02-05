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
        "//src/common:buffer_chain_lib",
        "//src/common:chained_stream_lib",
        ":embedded_metrics_message_lib",
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
        ":embedded_metrics_message_lib",
    ],
)

lightstep_cc_library(
    name = "embedded_metrics_message_lib",
    private_hdrs = [
        "embedded_metrics_message.h",
    ],
    srcs = [
        "embedded_metrics_message.cpp",
    ],
    deps = [
        "//lightstep-tracer-common:collector_proto_cc",
        "//src/common:protobuf_lib",
    ],
)
