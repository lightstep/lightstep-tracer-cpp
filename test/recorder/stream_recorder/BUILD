load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_catch_test",
    "lightstep_cc_library",
    "lightstep_package",
)

lightstep_package()

lightstep_catch_test(
    name = "stream_recorder_test",
    srcs = [
        "stream_recorder_test.cpp",
    ],
    deps = [
        "//src/tracer:lightstep_tracer_impl_lib",
        "//src/recorder/stream_recorder:stream_recorder_lib",
    ],
)

lightstep_catch_test(
    name = "utility_test",
    srcs = [
        "utility_test.cpp",
    ],
    deps = [
        "//src/recorder/stream_recorder:utility_lib",
    ],
)
