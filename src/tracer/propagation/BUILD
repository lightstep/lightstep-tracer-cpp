load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_cc_library",
    "lightstep_package",
)

lightstep_package()

lightstep_cc_library(
    name = "utility_lib",
    private_hdrs = [
        "utility.h",
    ],
    deps = [
        "//src/common:utility_lib",
    ],
    external_deps = [
        "@io_opentracing_cpp//:opentracing",
    ],
)

lightstep_cc_library(
    name = "propagator_interface",
    private_hdrs = [
        "propagator.h",
    ],
    deps = [
        "//src/tracer:baggage_flat_map_lib",
        ":trace_context_lib",
    ],
    external_deps = [
        "@com_google_protobuf//:protobuf",
        "@io_opentracing_cpp//:opentracing",
    ],
)

lightstep_cc_library(
    name = "multiheader_propagator_lib",
    private_hdrs = [
        "multiheader_propagator.h",
    ],
    srcs = [
        "multiheader_propagator.cpp",
    ],
    deps = [
        "//src/common:hex_conversion_lib",
        "//src/common:utility_lib",
        ":propagator_interface",
    ],
)

lightstep_cc_library(
    name = "lightstep_propagator_lib",
    private_hdrs = [
        "lightstep_propagator.h",
    ],
    srcs = [
        "lightstep_propagator.cpp",
    ],
    deps = [
        ":multiheader_propagator_lib",
    ],
)

lightstep_cc_library(
    name = "b3_propagator_lib",
    private_hdrs = [
        "b3_propagator.h",
    ],
    srcs = [
        "b3_propagator.cpp",
    ],
    deps = [
        ":lightstep_propagator_lib",
    ],
)

lightstep_cc_library(
    name = "envoy_propagator_lib",
    private_hdrs = [
        "envoy_propagator.h",
    ],
    srcs = [
        "envoy_propagator.cpp",
    ],
    deps = [
        "//3rd_party/base64:base64_lib",
        ":binary_propagation_lib",
        ":utility_lib",
    ],
)

lightstep_cc_library(
    name = "trace_context_lib",
    private_hdrs = [
      "trace_context.h",
    ],
    srcs = [
        "trace_context.cpp",
    ],
    deps = [
        "//src/common:hex_conversion_lib",
        "@io_opentracing_cpp//:opentracing",
    ],
)

lightstep_cc_library(
    name = "trace_context_propagator_lib",
    private_hdrs = [
        "trace_context_propagator.h",
    ],
    srcs = [
        "trace_context_propagator.cpp",
    ],
    deps = [
        "//src/common:hex_conversion_lib",
        "//src/common:utility_lib",
        ":propagator_interface",
    ],
)

lightstep_cc_library(
    name = "baggage_propagator_lib",
    private_hdrs = [
        "baggage_propagator.h",
    ],
    srcs = [
        "baggage_propagator.cpp",
    ],
    deps = [
        ":propagator_interface",
    ],
)

lightstep_cc_library(
    name = "binary_propagation_lib",
    private_hdrs = [
        "binary_propagation.h",
    ],
    srcs = [
        "binary_propagation.cpp",
    ],
    deps = [
        "//src/common:in_memory_stream_lib",
        "//src/common:utility_lib",
        "//lightstep-tracer-common:lightstep_carrier_proto_cc",
        "//src/tracer:baggage_flat_map_lib",
        ":propagator_interface",
    ],
    external_deps = [
        "@com_google_protobuf//:protobuf",
        "@io_opentracing_cpp//:opentracing",
    ],
)

lightstep_cc_library(
    name = "propagation_options_lib",
    private_hdrs = [
        "propagation_options.h",
    ],
    srcs = [
        "propagation_options.cpp",
    ],
    deps = [
        ":lightstep_propagator_lib",
        ":b3_propagator_lib",
        ":envoy_propagator_lib",
        ":trace_context_propagator_lib",
        ":baggage_propagator_lib",
        ":cloud_trace_propagator_lib",
    ],
)

lightstep_cc_library(
    name = "propagation_lib",
    private_hdrs = [
        "propagation.h",
    ],
    srcs = [
        "propagation.cpp",
    ],
    deps = [
        "//src/common:in_memory_stream_lib",
        ":propagation_options_lib",
        ":trace_context_lib",
    ],
)

lightstep_cc_library(
    name = "cloud_trace_propagator_lib",
    private_hdrs = [
        "cloud_trace_propagator.h",
    ],
    srcs = [
        "cloud_trace_propagator.cpp",
    ],
    deps = [
      "//3rd_party/base64:base64_lib",
      ":binary_propagation_lib",
      ":utility_lib",
    ],
)
