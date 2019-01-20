load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_cc_library",
    "lightstep_package",
)

lightstep_package()

lightstep_cc_library(
    name = "propagation_lib",
    hdrs = [
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
    name = "dynamic_load_lib",
    srcs = [
        "dynamic_load.cpp",
    ],
    deps = [
        ":lightstep_tracer_factory_lib",
    ],
)

lightstep_cc_library(
    name = "lightstep_span_context_interface",
    hdrs = [
        "lightstep_span_context.h",
    ],
    srcs = [
        "lightstep_span_context.cpp",
    ],
    deps = [
        ":propagation_lib",
    ],
)

lightstep_cc_library(
    name = "lightstep_immutable_span_context_lib",
    hdrs = [
        "lightstep_immutable_span_context.h",
    ],
    srcs = [
        "lightstep_immutable_span_context.cpp",
    ],
    deps = [
        ":lightstep_span_context_interface",
    ],
)

lightstep_cc_library(
    name = "lightstep_span_lib",
    hdrs = [
        "lightstep_span.h",
    ],
    srcs = [
        "lightstep_span.cpp",
    ],
    deps = [
        "//src/common:logger_lib",
        "//src/recorder:recorder_interface",
        ":lightstep_span_context_interface",
    ],
)

lightstep_cc_library(
    name = "lightstep_tracer_impl_lib",
    hdrs = [
        "lightstep_tracer_impl.h",
    ],
    srcs = [
        "lightstep_tracer_impl.cpp",
    ],
    deps = [
        "//src/common:logger_lib",
        "//src/recorder:recorder_interface",
        ":propagation_lib",
        ":lightstep_span_lib",
        ":lightstep_immutable_span_context_lib",
    ],
)

lightstep_cc_library(
    name = "lightstep_tracer_factory_lib",
    hdrs = [
        "lightstep_tracer_factory.h",
    ],
    srcs = [
        "lightstep_tracer_factory.cpp",
    ],
    deps = [
        "//lightstep-tracer-configuration:tracer_configuration_proto_cc",
        "//include/lightstep:tracer_interface",
    ],
    external_deps = [
        "@com_google_protobuf//:protobuf",
    ],
)
