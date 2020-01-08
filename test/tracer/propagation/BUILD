load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_catch_test",
    "lightstep_cc_test",
    "lightstep_portable_cc_binary",
    "lightstep_cc_library",
    "lightstep_package",
)

lightstep_package()

lightstep_cc_library(
    name = "text_map_carrier_lib",
    private_hdrs = [
        "text_map_carrier.h",
    ],
    srcs = [
        "text_map_carrier.cpp",
    ],
    external_deps = [
        "@io_opentracing_cpp//:opentracing",
    ],
)

lightstep_cc_library(
    name = "http_headers_carrier_lib",
    private_hdrs = [
        "http_headers_carrier.h",
    ],
    srcs = [
        "http_headers_carrier.cpp",
    ],
    external_deps = [
        "@io_opentracing_cpp//:opentracing",
    ],
)

lightstep_cc_library(
    name = "utility_lib",
    private_hdrs = [
        "utility.h",
    ],
    srcs = [
        "utility.cpp",
    ],
    deps = [
        "//src/tracer:lightstep_span_context_interface",
        "//src/tracer:immutable_span_context_lib",
    ],
    external_deps = [
        "@io_opentracing_cpp//:opentracing",
    ],
)

lightstep_catch_test(
    name = "propagation_test",
    srcs = [
        "propagation_test.cpp",
    ],
    deps = [
        "//:manual_tracer_lib",
        "//test/recorder:in_memory_recorder_lib",
        ":text_map_carrier_lib",
        ":http_headers_carrier_lib",
        ":utility_lib",
    ],
)

lightstep_catch_test(
    name = "envoy_propagation_test",
    srcs = [
        "envoy_propagation_test.cpp",
    ],
    deps = [
        "//:manual_tracer_lib",
        "//test/recorder:in_memory_recorder_lib",
        ":text_map_carrier_lib",
        ":http_headers_carrier_lib",
        ":utility_lib",
    ],
)

lightstep_catch_test(
    name = "b3_propagation_test",
    srcs = [
        "b3_propagation_test.cpp",
    ],
    deps = [
        "//:manual_tracer_lib",
        "//test/recorder:in_memory_recorder_lib",
        ":text_map_carrier_lib",
        ":http_headers_carrier_lib",
        ":utility_lib",
    ],
)

lightstep_catch_test(
    name = "trace_context_test",
    srcs = [
        "trace_context_test.cpp",
    ],
    deps = [
        "//src/tracer/propagation:trace_context_lib",
    ],
)

lightstep_catch_test(
    name = "lightstep_propagation_test",
    srcs = [
        "lightstep_propagation_test.cpp",
    ],
    deps = [
        "//:manual_tracer_lib",
        "//test/recorder:in_memory_recorder_lib",
        ":text_map_carrier_lib",
        ":http_headers_carrier_lib",
        ":utility_lib",
    ],
)

lightstep_catch_test(
    name = "trace_context_propagation_test",
    srcs = [
        "trace_context_propagation_test.cpp",
    ],
    deps = [
        "//:manual_tracer_lib",
        "//test/recorder:in_memory_recorder_lib",
        ":text_map_carrier_lib",
        ":http_headers_carrier_lib",
        ":utility_lib",
    ],
)

lightstep_catch_test(
    name = "propagation_options_test",
    srcs = [
        "propagation_options_test.cpp",
    ],
    deps = [
        "//src/tracer/propagation:propagation_lib",
    ],
)
