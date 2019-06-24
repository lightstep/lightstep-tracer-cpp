load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_cc_library",
    "lightstep_package",
)

lightstep_package()

lightstep_cc_library(
    name = "propagation_lib",
    private_hdrs = [
        "propagation.h",
    ],
    srcs = [
        "propagation.cpp",
    ],
    deps = [
        "//src/common:utility_lib",
        "//src/common:in_memory_stream_lib",
        "//3rd_party/base64:base64_lib",
        "//lightstep-tracer-common:lightstep_carrier_proto_cc",
    ],
    external_deps = [
        "@com_google_protobuf//:protobuf",
        "@io_opentracing_cpp//:opentracing",
    ],
)

lightstep_cc_library(
    name = "lightstep_span_lib",
    private_hdrs = [
        "lightstep_span.h",
    ],
    srcs = [
        "lightstep_span.cpp",
    ],
    deps = [
        "//src/common:logger_lib",
        "//src/common:utility_lib",
        "//src/common:random_lib",
        "//src/recorder:recorder_interface",
        "//src/tracer:lightstep_span_context_interface",
        "//src/tracer:utility_lib",
    ],
)

lightstep_cc_library(
    name = "lightstep_immutable_span_context_lib",
    private_hdrs = [
        "lightstep_immutable_span_context.h",
    ],
    srcs = [
        "lightstep_immutable_span_context.cpp",
    ],
    deps = [
        "//src/tracer:lightstep_span_context_interface",
    ],
)

lightstep_cc_library(
    name = "lightstep_tracer_impl_lib",
    private_hdrs = [
        "lightstep_tracer_impl.h",
    ],
    srcs = [
        "lightstep_tracer_impl.cpp",
    ],
    deps = [
        "//src/common:logger_lib",
        "//src/recorder:recorder_interface",
        ":lightstep_span_lib",
        ":propagation_lib",
        ":lightstep_immutable_span_context_lib",
    ],
)

