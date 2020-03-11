load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_cc_library",
    "lightstep_package",
)

lightstep_package()

lightstep_cc_library(
    name = "atomic_unique_ptr_lib",
    private_hdrs = [
        "atomic_unique_ptr.h",
    ],
)

lightstep_cc_library(
    name = "buffer_chain_lib",
    srcs = [
        "buffer_chain.cpp",
    ],
    deps = [
        "//include/lightstep:buffer_chain_interface",
    ],
)

lightstep_cc_library(
    name = "spin_lock_mutex_lib",
    private_hdrs = [
        "spin_lock_mutex.h",
    ],
)

lightstep_cc_library(
    name = "direct_coded_output_stream_lib",
    private_hdrs = [
        "direct_coded_output_stream.h",
    ],
    external_deps = [
        "@com_google_protobuf//:protobuf",
    ],
)

lightstep_cc_library(
    name = "chained_stream_lib",
    private_hdrs = [
        "chained_stream.h",
    ],
    srcs = [
        "chained_stream.cpp",
    ],
    deps = [
        ":fragment_input_stream_lib",
        ":noncopyable_lib",
        ":composable_fragment_input_stream_lib",
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
    deps = [
        ":circular_buffer_range_lib",
        ":atomic_unique_ptr_lib",
    ],
)

lightstep_cc_library(
    name = "circular_buffer_range_lib",
    private_hdrs = [
        "circular_buffer_range.h",
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
    name = "fast_random_number_generator_lib",
    private_hdrs = [
        "fast_random_number_generator.h",
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
        "//src/common/platform:fork_lib",
        ":fast_random_number_generator_lib",
    ],
    linkopts = [
        "-pthread",
    ],
)

lightstep_cc_library(
    name = "flat_map_lib",
    private_hdrs = [
        "flat_map.h",
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
    name = "hex_conversion_lib",
    private_hdrs = [
        "hex_conversion.h",
    ],
    srcs = [
        "hex_conversion.cpp",
    ],
    external_deps = [
        "@io_opentracing_cpp//:opentracing",
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
        "//src/common/platform:time_lib",
        "//lightstep-tracer-common:collector_proto_cc",
        ":logger_lib",
        ":hex_conversion_lib",
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
    name = "composable_fragment_input_stream_lib",
    private_hdrs = [
        "composable_fragment_input_stream.h",
    ],
    srcs = [
        "composable_fragment_input_stream.cpp",
    ],
    deps = [
        ":fragment_input_stream_lib",
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

lightstep_cc_library(
    name = "serialization_lib",
    private_hdrs = [
        "serialization.h",
    ],
    srcs = [
        "serialization.cpp",
    ],
    deps = [
        ":utility_lib",
        ":direct_coded_output_stream_lib",
    ],
    external_deps = [
        "@com_google_protobuf//:protobuf",
        "@io_opentracing_cpp//:opentracing",
    ],
)

lightstep_cc_library(
    name = "report_request_framing_lib",
    private_hdrs = [
        "report_request_framing.h",
    ],
    srcs = [
        "report_request_framing.cpp",
    ],
    deps = [
        ":serialization_lib",
    ],
)

lightstep_cc_library(
    name = "chunked_http_framing_lib",
    private_hdrs = [
        "chunked_http_framing.h",
    ],
    srcs = [
        "chunked_http_framing.cpp",
    ],
    deps = [
        ":serialization_lib",
        ":hex_conversion_lib",
    ],
    external_deps = [
        "@io_opentracing_cpp//:opentracing",
    ],
)

lightstep_cc_library(
    name = "timestamp_lib",
    private_hdrs = [
        "timestamp.h",
    ],
    srcs = [
        "timestamp.cpp",
    ],
)
