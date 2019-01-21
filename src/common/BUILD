load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_cc_library",
    "lightstep_package",
)

lightstep_package()

lightstep_cc_library(
  name = "atomic_bit_set_lib",
  hdrs = [
      "atomic_bit_set.h",
  ],
  srcs = [
      "atomic_bit_set.cpp",
  ],
)

lightstep_cc_library(
    name = "bipart_memory_stream_lib",
    hdrs = [
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
    hdrs = [
        "circular_buffer.h",
    ],
    srcs = [
        "circular_buffer.cpp",
    ],
)

lightstep_cc_library(
    name = "chunk_circular_buffer_lib",
    hdrs = [
      "chunk_circular_buffer.h",
    ],
    srcs = [
        "chunk_circular_buffer.cpp",
    ],
    deps = [
        ":atomic_bit_set_lib",
        ":bipart_memory_stream_lib",
        ":circular_buffer_lib",
        ":utility_lib",
    ],
)

lightstep_cc_library(
    name = "in_memory_stream_lib",
    hdrs = [
        "in_memory_stream.h",
    ],
    srcs = [
        "in_memory_stream.cpp",
    ],
)

lightstep_cc_library(
    name = "logger_lib",
    hdrs = [
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
    name = "utility_lib",
    hdrs = [
        "utility.h",
    ],
    srcs = [
        "utility.cpp",
    ],
    deps = [
        "//lightstep-tracer-common:collector_proto_cc",
        ":logger_lib",
        "//3rd_party/randutils:randutils_lib",
    ],
    external_deps = [
        "@com_google_protobuf//:protobuf",
        "@io_opentracing_cpp//:opentracing",
    ],
)

lightstep_cc_library(
    name = "condition_variable_wrapper_lib",
    hdrs = [
        "condition_variable_wrapper.h",
    ]
)