load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_cc_library",
    "lightstep_package",
)

lightstep_package()

lightstep_cc_library(
  name = "atomic_bit_set_lib",
  private_hdrs = [
      "atomic_bit_set.h",
  ],
  srcs = [
      "atomic_bit_set.cpp",
  ],
)

lightstep_cc_library(
    name = "bipart_memory_stream_lib",
    private_hdrs = [
        "bipart_memory_stream.h",
    ],
    srcs = [
        "bipart_memory_stream.cpp",
    ],
    external_deps = [
        "@com_google_protobuf//:protobuf",
    ],
)

lightstep_cc_library(
    name = "circular_buffer_lib",
    private_hdrs = [
        "circular_buffer.h",
    ],
    srcs = [
        "circular_buffer.cpp",
    ],
)

lightstep_cc_library(
    name = "chunk_circular_buffer_lib",
    private_hdrs = [
      "chunk_circular_buffer.h",
    ],
    srcs = [
        "chunk_circular_buffer.cpp",
    ],
    deps = [
        ":atomic_bit_set_lib",
        ":bipart_memory_stream_lib",
        ":circular_buffer_lib",
        ":function_ref_lib",
        ":utility_lib",
    ],
)

lightstep_cc_library(
    name = "function_ref_lib",
    private_hdrs = [
        "function_ref.h",
    ],
)

lightstep_cc_library(
    name = "in_memory_stream_lib",
    private_hdrs = [
        "in_memory_stream.h",
    ],
    srcs = [
        "in_memory_stream.cpp",
    ],
)

lightstep_cc_library(
    name = "logger_lib",
    private_hdrs = [
        "logger.h",
    ],
    srcs = [
        "logger.cpp",
    ],
    deps = [
        "//include/lightstep:tracer_interface",
    ],
)

lightstep_cc_library(
    name = "noncopyable_lib",
    private_hdrs = [
        "noncopyable.h",
    ],
)

lightstep_cc_library(
    name = "random_lib",
    private_hdrs = [
        "random.h",
    ],
    srcs = [
        "random.cpp",
    ],
    deps = [
        "//3rd_party/randutils:randutils_lib",
    ],
    linkopts = [
        "-pthread",
    ],
)

lightstep_cc_library(
    name = "protobuf_lib",
    private_hdrs = [
        "protobuf.h",
    ],
    srcs = [
        "protobuf.cpp",
    ],
    external_deps = [
        "@com_google_protobuf//:protobuf",
    ],
)

lightstep_cc_library(
    name = "utility_lib",
    private_hdrs = [
        "utility.h",
    ],
    srcs = [
        "utility.cpp",
    ],
    deps = [
        "//lightstep-tracer-common:collector_proto_cc",
        ":logger_lib",
    ],
    external_deps = [
        "@com_google_protobuf//:protobuf",
        "@io_opentracing_cpp//:opentracing",
    ],
)

lightstep_cc_library(
    name = "condition_variable_wrapper_lib",
    private_hdrs = [
        "condition_variable_wrapper.h",
    ]
)

lightstep_cc_library(
    name = "random_traverser_lib",
    private_hdrs = [
        "random_traverser.h",
    ],
    srcs = [
        "random_traverser.cpp",
    ],
    deps = [
        ":function_ref_lib",
        ":random_lib",
    ],
)

lightstep_cc_library(
    name = "fragment_input_stream_lib",
    private_hdrs = [
        "fragment_input_stream.h",
    ],
    srcs = [
        "fragment_input_stream.cpp",
    ],
    deps = [
        "//src/common:function_ref_lib",
    ],
)

lightstep_cc_library(
    name = "fragment_array_input_stream_lib",
    private_hdrs = [
        "fragment_array_input_stream.h",
    ],
    srcs = [
        "fragment_array_input_stream.cpp",
    ],
    deps = [
        ":fragment_input_stream_lib",
    ],
)

