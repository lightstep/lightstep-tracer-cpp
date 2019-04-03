load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_cc_binary",
)

lightstep_cc_binary(
    name = "tutorial",
    srcs = [
        "text_map_carrier.h",
        "tutorial.cpp",
    ],
    deps = [
        "//:tracer_lib",
    ],
)

