load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_catch_test",
    "lightstep_cc_test",
    "lightstep_portable_cc_binary",
    "lightstep_cc_library",
    "lightstep_package",
)

lightstep_package()

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

lightstep_catch_test(
    name = "utility_test",
    srcs = [
        "utility_test.cpp",
    ],
    deps = [
        "//src/tracer:utility_lib",
    ],
)

lightstep_catch_test(
    name = "dynamic_load_test",
    srcs = [
        "dynamic_load_test.cpp",
    ],
    data = [
        "//:liblightstep_tracer_plugin.so",
    ],
    deps = [
        "//test/mock_satellite:mock_satellite_lib",
        "//test:ports_lib",
        "//test:utility_lib",
    ],
    external_deps = [
        "@io_opentracing_cpp//:opentracing",
    ],
)

lightstep_catch_test(
    name = "serialization_test",
    srcs = [
        "serialization_test.cpp",
    ],
    deps = [
        "//src/tracer:serialization_lib",
        "//src/common:utility_lib",
    ],
)

lightstep_catch_test(
    name = "json_options_test",
    srcs = [
        "json_options_test.cpp",
    ],
    deps = [
         "//src/tracer:json_options_lib",
    ],
)

lightstep_portable_cc_binary(
    name = "span_probe",
    srcs = [
        "span_probe.cpp",
    ],
    external_deps = [
        "@io_opentracing_cpp//:opentracing",
    ],
)
