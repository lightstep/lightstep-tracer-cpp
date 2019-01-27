load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_google_benchmark",
    "lightstep_package",
)

lightstep_google_benchmark(
    name = "span_operations_benchmark",
    srcs = [
        "span_operations_benchmark.cpp",
    ],
    deps = [
        "//:manual_tracer_lib",
    ],
)

