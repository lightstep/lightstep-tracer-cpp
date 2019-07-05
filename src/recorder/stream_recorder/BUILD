load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_cc_library",
    "lightstep_package",
)

lightstep_package()

lightstep_cc_library(
    name = "stream_recorder_options_lib",
    private_hdrs = [
        "stream_recorder_options.h",
    ],
    deps = [
        "//src/network:dns_resolver_interface",
    ],
)

lightstep_cc_library(
    name = "stream_recorder_metrics_lib",
    private_hdrs = [
        "stream_recorder_metrics.h",
    ],
    srcs = [
      "stream_recorder_metrics.cpp",
    ],
    deps = [
        "//include/lightstep:metrics_observer_interface",
    ],
)

lightstep_cc_library(
    name = "span_stream_lib",
    private_hdrs = [
        "span_stream.h",
    ],
    srcs = [
        "span_stream.cpp",
    ],
    deps = [
        "//src/common:chunk_circular_buffer_lib",
        "//src/common:fragment_array_input_stream_lib",
        ":utility_lib",
        ":fragment_span_input_stream_lib",
        ":stream_recorder_metrics_lib",
    ],
)

lightstep_cc_library(
    name = "span_stream2_lib",
    private_hdrs = [
        "span_stream2.h",
    ],
    srcs = [
        "span_stream2.cpp",
    ],
    deps = [
        "//src/common:circular_buffer2_lib",
        "//src/common:serialization_chain_lib",
        "//src/common:fragment_input_stream_lib",
        ":stream_recorder_metrics_lib",
    ],
)

lightstep_cc_library(
    name = "fragment_span_input_stream_lib",
    private_hdrs = [
        "fragment_span_input_stream.h",
    ],
    srcs = [
        "fragment_span_input_stream.cpp",
    ],
    deps = [
        "//src/common:circular_buffer_lib",
        "//src/common:fragment_array_input_stream_lib",
        ":utility_lib",
    ],
)

lightstep_cc_library(
    name = "connection_stream_lib",
    private_hdrs = [
        "connection_stream.h",
    ],
    srcs = [
        "connection_stream.cpp",
    ],
    deps = [
        "//src/common:function_ref_lib",
        "//src/common:utility_lib",
        "//src/common:fragment_input_stream_lib",
        "//src/common:fragment_array_input_stream_lib",
        ":embedded_metrics_message_lib",
        ":span_stream_lib",
        ":stream_recorder_metrics_lib",
        ":fragment_span_input_stream_lib",
    ],
)


lightstep_cc_library(
    name = "connection_stream2_lib",
    private_hdrs = [
        "connection_stream2.h",
    ],
    srcs = [
        "connection_stream2.cpp",
    ],
    deps = [
        "//src/common:function_ref_lib",
        "//src/common:utility_lib",
        "//src/common:fragment_input_stream_lib",
        "//src/common:fragment_array_input_stream_lib",
        ":embedded_metrics_message_lib",
        ":span_stream2_lib",
        ":stream_recorder_metrics_lib",
        ":fragment_span_input_stream_lib",
    ],
)

lightstep_cc_library(
    name = "stream_recorder_lib",
    private_hdrs = [
        "stream_recorder.h",
        "stream_recorder2.h"
    ],
    srcs = [
        "stream_recorder.cpp",
        "stream_recorder2.cpp",
        "stream_recorder_impl.h",
        "stream_recorder_impl.cpp",
        "stream_recorder_impl2.h",
        "stream_recorder_impl2.cpp",
    ],
    deps = [
        "//src/common:chunk_circular_buffer_lib",
        "//src/common:circular_buffer2_lib",
        "//src/common:logger_lib",
        "//src/common:noncopyable_lib",
        "//src/common:protobuf_lib",
        "//src/common:serialization_chain_lib",
        "//src/network:event_lib",
        "//src/network:timer_event_lib",
        "//src/recorder:fork_aware_recorder_lib",
        "//src/recorder:stream_recorder_interface",
        ":stream_recorder_options_lib",
        ":stream_recorder_metrics_lib",
        ":satellite_streamer_lib",
    ],
)

lightstep_cc_library(
    name = "satellite_dns_resolution_manager_lib",
    private_hdrs = [
        "satellite_dns_resolution_manager.h",
    ],
    srcs = [
        "satellite_dns_resolution_manager.cpp",
    ],
    deps = [
        "//src/common:logger_lib",
        "//src/common:random_lib",
        "//src/common:noncopyable_lib",
        "//src/network:event_lib",
        "//src/network:timer_event_lib",
        "//src/network:dns_resolver_interface",
        ":stream_recorder_options_lib",
    ],
)

lightstep_cc_library(
    name = "satellite_endpoint_manager_lib",
    private_hdrs = [
        "satellite_endpoint_manager.h",
    ],
    srcs = [
        "satellite_endpoint_manager.cpp",
    ],
    deps = [
        ":satellite_dns_resolution_manager_lib",
        ":utility_lib",
        "//src/network:dns_resolver_interface",
        "//src/common:noncopyable_lib",
    ],
)

lightstep_cc_library(
    name = "satellite_streamer_lib",
    private_hdrs = [
        "satellite_connection.h",
        "satellite_streamer.h",
    ],
    srcs = [
        "satellite_connection.cpp",
        "satellite_streamer.cpp",
    ],
    deps = [
        ":satellite_endpoint_manager_lib",
        "//src/common:chunk_circular_buffer_lib",
        "//src/common:noncopyable_lib",
        "//src/common:random_lib",
        "//src/common:random_traverser_lib",
        "//src/network:socket_lib",
        "//src/network:event_lib",
        "//src/network:timer_event_lib",
        "//src/network:vector_write_lib",
        ":stream_recorder_metrics_lib",
        ":embedded_metrics_message_lib",
        ":utility_lib",
        ":host_header_lib",
        ":span_stream_lib",
        ":connection_stream_lib",
        ":status_line_parser_lib",
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
        "//include/lightstep:tracer_interface",
        "//lightstep-tracer-common:collector_proto_cc",
        "//src/common:protobuf_lib",
        "//src/common:utility_lib",
        "//src/common:circular_buffer_lib",
        "//src/common:fragment_array_input_stream_lib",
    ],
)

lightstep_cc_library(
    name = "embedded_metrics_message_lib",
    private_hdrs = [
        "embedded_metrics_message.h",
    ],
    srcs = [
        "embedded_metrics_message.cpp",
    ],
    deps = [
        "//lightstep-tracer-common:collector_proto_cc",
        "//src/common:protobuf_lib",
    ],
)

lightstep_cc_library(
    name = "host_header_lib",
    private_hdrs = [
      "host_header.h",
    ],
    srcs = [
      "host_header.cpp",
    ],
    deps = [
        "//include/lightstep:tracer_interface",
    ],
)

lightstep_cc_library(
    name = "status_line_parser_lib",
    private_hdrs = [
        "status_line_parser.h",
    ],
    srcs = [
        "status_line_parser.cpp",
    ],
    deps = [
    ],
    external_deps = [
        "@io_opentracing_cpp//:opentracing",
    ],
)
