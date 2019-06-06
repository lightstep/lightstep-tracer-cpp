load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_catch_test",
    "lightstep_cc_test",
    "lightstep_package",
)

lightstep_package()

lightstep_catch_test(
    name = "atomic_bit_set_test",
    srcs = [
        "atomic_bit_set_test.cpp",
    ],
    deps = [
        "//src/common:atomic_bit_set_lib",
    ],
)

lightstep_catch_test(
    name = "bipart_memory_stream_test",
    srcs = [
        "bipart_memory_stream_test.cpp",
    ],
    deps = [
        "//src/common:bipart_memory_stream_lib",
    ],
)

lightstep_catch_test(
    name = "circular_buffer_test",
    srcs = [
        "circular_buffer_test.cpp",
    ],
    deps = [
        "//src/common:circular_buffer_lib",
    ],
)

lightstep_catch_test(
    name = "chunk_circular_buffer_test",
    srcs = [
        "chunk_circular_buffer_test.cpp",
    ],
    deps = [
        "//src/common:chunk_circular_buffer_lib",
        "//src/common:utility_lib",
        "//test:utility_lib",
        "//test:number_simulation_lib",
    ],
)

lightstep_catch_test(
    name = "function_ref_test",
    srcs = [
        "function_ref_test.cpp",
    ],
    deps = [
        "//src/common:function_ref_lib",
    ],
)

lightstep_catch_test(
    name = "utility_test",
    srcs = [
        "utility_test.cpp",
    ],
    deps = [
        "//src/common:utility_lib",
        "//src/common:bipart_memory_stream_lib",
    ],
)

lightstep_catch_test(
    name = "logger_test",
    srcs = [
        "logger_test.cpp",
    ],
    deps = [
        "//src/common:logger_lib",
    ],
)

lightstep_catch_test(
    name = "random_test",
    srcs = [
        "random_test.cpp",
    ],
    deps = [
        "//src/common:random_lib",
    ],
)

lightstep_cc_test(
    name = "fork_id_test",
    srcs = [
        "fork_id_test.cpp",
    ],
    deps = [
        "//src/common:random_lib",
    ],
)

lightstep_catch_test(
    name = "protobuf_test",
    srcs = [
        "protobuf_test.cpp",
    ],
    deps = [
        "//src/common:protobuf_lib",
        "//lightstep-tracer-common:collector_proto_cc",
    ],
)

lightstep_catch_test(
    name = "random_traverser_test",
    srcs = [
        "random_traverser_test.cpp",
    ],
    deps = [
      "//src/common:random_traverser_lib",
    ],
)

lightstep_catch_test(
    name = "fragment_input_stream_test",
    srcs = [
        "fragment_input_stream_test.cpp",
    ],
    deps = [
        "//src/common:fragment_array_input_stream_lib",
        "//test:utility_lib",
    ],
)

lightstep_catch_test(
    name = "fragment_array_input_stream_test",
    srcs = [
        "fragment_array_input_stream_test.cpp",
    ],
    deps = [
        "//src/common:fragment_array_input_stream_lib",
        "//test:utility_lib",
    ],
)

