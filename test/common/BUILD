load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_catch_test",
    "lightstep_cc_test",
    "lightstep_package",
)

lightstep_package()

lightstep_catch_test(
    name = "atomic_unique_ptr_test",
    srcs = [
        "atomic_unique_ptr_test.cpp",
    ],
    deps = [
        "//src/common:atomic_unique_ptr_lib",
    ],
)

lightstep_catch_test(
    name = "circular_buffer_test",
    srcs = [
        "circular_buffer_test.cpp",
    ],
    linkopts = ["-pthread"],
    deps = [
        "//src/common:circular_buffer_lib",
    ],
)

lightstep_catch_test(
    name = "circular_buffer_range_test",
    srcs = [
        "circular_buffer_range_test.cpp",
    ],
    deps = [
        "//src/common:circular_buffer_range_lib",
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
    name = "flat_map_test",
    srcs = [
        "flat_map_test.cpp",
    ],
    deps = [
        "//src/common:flat_map_lib",
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

lightstep_catch_test(
    name = "serialization_chain_test",
    srcs = [
        "serialization_chain_test.cpp",
    ],
    deps = [
        "//src/common:serialization_chain_lib",
        "//test:utility_lib",
    ],
)

proto_library(
  name = "test_proto",    
  srcs = ["test.proto"],
  deps = [
      "//lightstep-tracer-common:collector_proto",
        "@com_google_protobuf//:timestamp_proto",
  ],
)

cc_proto_library(
    name = "test_proto_cc",
    deps = [":test_proto"],
)

lightstep_catch_test(
    name = "serialization_test",
    srcs = [
        "serialization_test.cpp",
    ],
    deps = [
        "//src/common:serialization_lib",
        "//src/common:utility_lib",
        ":test_proto_cc",
    ],
)

lightstep_catch_test(
    name = "timestamp_test",
    srcs = [
        "timestamp_test.cpp",
    ],
    deps = [
        "//src/common:timestamp_lib",
    ],
)
