load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_cc_library",
    "lightstep_package",
)

lightstep_package()

lightstep_cc_library(
    name = "zero_copy_stream_lib",
    private_hdrs = [
        "zero_copy_stream.h",
    ],
    external_deps = [
        "@com_google_protobuf//:protobuf",
    ],
)

lightstep_cc_library(
    name = "coded_stream_lib",
    private_hdrs = [
        "coded_stream.h",
    ],
    srcs = [
        "coded_stream.cc",
    ],
    deps = [
        ":zero_copy_stream_lib",
    ],
)
