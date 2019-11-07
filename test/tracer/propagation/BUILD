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

lightstep_catch_test(
    name = "propagation_test",
    srcs = [
        "propagation_test.cpp",
    ],
    deps = [
        "//:manual_tracer_lib",
        "//test/recorder:in_memory_recorder_lib",
        ":text_map_carrier_lib",
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
