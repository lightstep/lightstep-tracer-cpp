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
        "//src/network:fragment_array_input_stream_lib",
        ":utility_lib",
        ":fragment_span_input_stream_lib",
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
        "//src/network:fragment_array_input_stream_lib",
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
        "//src/network:fragment_input_stream_lib",
        "//src/network:fragment_array_input_stream_lib",
        ":embedded_metrics_message_lib",
        ":span_stream_lib",
        ":stream_recorder_metrics_lib",
        ":fragment_span_input_stream_lib",
    ],
)

lightstep_cc_library(
    name = "stream_recorder_lib",
    private_hdrs = [
        "stream_recorder.h",
    ],
    srcs = [
        "stream_recorder.cpp",
    ],
    deps = [
        "//src/common:logger_lib",
        "//src/common:noncopyable_lib",
        "//src/common:protobuf_lib",
        "//src/common:chunk_circular_buffer_lib",
        "//src/network:event_lib",
        "//src/network:timer_event_lib",
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
        "//src/network:fragment_array_input_stream_lib",
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
