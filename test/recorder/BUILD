load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_catch_test",
    "lightstep_cc_library",
    "lightstep_package",
)

lightstep_package()

lightstep_cc_library(
    name = "in_memory_recorder_lib",
    hdrs = [
        "in_memory_recorder.h",
    ],
    deps = [
        "//src/recorder:recorder_interface",
    ],
)
