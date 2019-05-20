load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_catch_test",
    "lightstep_cc_test",
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
    name = "tracer_test",
    srcs = [
        "tracer_test.cpp",
    ],
    deps = [
        "//:tracer_lib",
        "//test/recorder:in_memory_recorder_lib",
        "//test:utility_lib",
    ],
)

lightstep_cc_library(
    name = "in_memory_collector_lib",
    private_hdrs = [
        "in_memory_collector.h",
    ],
    srcs = [
        "in_memory_collector.cpp",
    ],
    deps = [
        "//src/recorder/grpc_transporter:collector_proto_grpc",
    ],
)

cc_binary(
    name = "lightstep_plugin.so",
    linkshared = 1,
    linkopts = [
        "-Wl,--version-script=$(location :tracer-export-map.ld)",
    ],
    deps = [
        ":tracer-export-map.ld",
        "//:tracer_lib",
    ],
)

lightstep_catch_test(
    name = "dynamic_load_test",
    srcs = [
        "dynamic_load_test.cpp",
    ],
    data = [
        ":lightstep_plugin.so",
    ],
    deps = [
        ":in_memory_collector_lib",
    ],
    external_deps = [
        "@io_opentracing_cpp//:opentracing",
    ],
)
