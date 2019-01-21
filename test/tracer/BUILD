load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_catch_test",
    "lightstep_cc_test",
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
    name = "tracer_test",
    srcs = [
        "tracer_test.cpp",
    ],
    deps = [
        "//:manual_tracer_lib",
        "//test/recorder:in_memory_recorder_lib",
        "//test:utility_lib",
    ],
)
