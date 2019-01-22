load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_cc_library",
    "lightstep_package",
)

lightstep_package()

lightstep_cc_library(
    name = "stream_recorder_options_lib",
    private_hdrs = [
        "stream_recorder_options.h",
    ],
)

lightstep_cc_library(
    name = "stream_recorder_lib",
    private_hdrs = [
        "stream_recorder.h",
    ],
    srcs = [
        "stream_recorder.cpp",
    ],
    deps = [
        "//src/common:logger_lib",
        "//src/recorder:stream_recorder_interface",
        ":stream_recorder_options_lib",
    ],
)
