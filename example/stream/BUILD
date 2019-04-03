load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_cc_binary",
)

lightstep_cc_binary(
    name = "stream",
    srcs = [
        "main.cpp",
    ],
    deps = [
        "//:tracer_lib",
    ],
)
