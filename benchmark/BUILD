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
        "//:tracer_lib",
    ],
)

lightstep_google_benchmark(
    name = "manual_tracer_benchmark",
    srcs = [
        "manual_tracer_benchmark.cpp",
    ],
    deps = [
        "//:tracer_lib",
    ],
)

lightstep_google_benchmark(
    name = "buffer_benchmark",
    srcs = [
        "buffer_benchmark.cpp",
    ],
    deps = [
      "//src/common:circular_buffer_lib", 
      "//test:baseline_circular_buffer_lib",
    ],
)
