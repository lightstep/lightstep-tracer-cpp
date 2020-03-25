load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_cc_library",
    "lightstep_catch_test",
    "lightstep_package",
)

lightstep_package()

lightstep_cc_library(
    name = "baseline_circular_buffer_lib",
    private_hdrs = [
        "baseline_circular_buffer.h",
    ],
)

lightstep_catch_test(
    name = "baseline_circular_buffer_test",
    srcs = [
        "baseline_circular_buffer_test.cpp",
    ],
    deps = [
        ":baseline_circular_buffer_lib",
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
        "//src/common:utility_lib",
        "//src/network:socket_lib",
        "//src/common:fragment_input_stream_lib",
        "//src/common:report_request_framing_lib",
        "//src/common:serialization_lib",
        "//src/common:chained_stream_lib",
        "//src/common:circular_buffer_lib",
    ],
    external_deps = [
        "@io_opentracing_cpp//:opentracing",
    ],
)

lightstep_cc_library(
    name = "zero_copy_connection_input_stream_lib",
    private_hdrs = [
        "zero_copy_connection_input_stream.h",
    ],
    srcs = [
        "zero_copy_connection_input_stream.cpp",
    ],
    deps = [
        "//src/recorder/stream_recorder:connection_stream_lib",
    ],
    external_deps = [
        "@com_google_protobuf//:protobuf",
    ],
)


lightstep_cc_library(
    name = "number_simulation_lib",
    private_hdrs = [
        "number_simulation.h",
    ],
    srcs = [
        "number_simulation.cpp",
    ],
    deps = [
        "//src/common:circular_buffer_lib",
        "//src/common:utility_lib",
        "//src/recorder/stream_recorder:connection_stream_lib",
        ":utility_lib",
        ":zero_copy_connection_input_stream_lib",
    ],
    external_deps = [
        "@io_opentracing_cpp//:opentracing",
        "@com_google_protobuf//:protobuf",
    ],
)


lightstep_cc_library(
    name = "testing_condition_variable_wrapper_lib",
    private_hdrs = [
        "testing_condition_variable_wrapper.h",
    ],
    srcs = [
        "testing_condition_variable_wrapper.cpp",
    ],
    deps = [
        "//src/common:condition_variable_wrapper_lib",
    ],
)

lightstep_cc_library(
    name = "http_connection_lib",
    private_hdrs = [
        "http_connection.h"
    ],
    srcs = [
        "http_connection.cpp"
    ],
    deps = [
        "//src/common:noncopyable_lib",
        "//src/network:event_lib",
    ],
    external_deps = [
        "@com_github_libevent_libevent//:libevent",
        "@com_google_protobuf//:protobuf",
    ],
)

lightstep_cc_library(
    name = "child_process_handle_lib",
    private_hdrs = [
        "child_process_handle.h",
    ],
    srcs = [
        "child_process_handle.cpp",
    ],
)

lightstep_cc_library(
    name = "ports_lib",
    private_hdrs = [
        "ports.h",
    ],
)

lightstep_cc_library(
    name = "mock_dns_resolution_callback_lib",
    private_hdrs = [
        "mock_dns_resolution_callback.h",
    ],
    srcs = [
      "mock_dns_resolution_callback.cpp",
    ],
    deps = [
        "//src/network:dns_resolver_interface",
        "//src/network:event_lib",
    ],
)

lightstep_cc_library(
    name = "string_logger_sink_lib",
    private_hdrs = [
        "string_logger_sink.h",
    ],
    srcs = [
        "string_logger_sink.cpp",
    ],
    deps = [
        "//include/lightstep:tracer_interface",
    ],
)

lightstep_cc_library(
    name = "composable_fragment_input_stream_wrapper_lib",
    private_hdrs = [
        "composable_fragment_input_stream_wrapper.h",
    ],
    deps = [
        "//src/common:composable_fragment_input_stream_lib",
    ],
)
