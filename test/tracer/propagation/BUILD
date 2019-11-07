load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_catch_test",
    "lightstep_cc_test",
    "lightstep_portable_cc_binary",
    "lightstep_cc_library",
    "lightstep_package",
)

lightstep_package()

lightstep_catch_test(
    name = "propagation_test",
    srcs = [
        "propagation_test.cpp",
    ],
    deps = [
        "//:manual_tracer_lib",
        "//test/recorder:in_memory_recorder_lib",
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
