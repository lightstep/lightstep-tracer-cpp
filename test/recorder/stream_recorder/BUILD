load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_catch_test",
    "lightstep_cc_library",
    "lightstep_package",
)

lightstep_package()

lightstep_catch_test(
    name = "stream_recorder_test",
    srcs = [
        "stream_recorder_test.cpp",
    ],
    deps = [
        "//src/tracer/legacy:legacy_tracer_impl_lib",
        "//src/tracer:tracer_impl_lib",
        "//src/tracer:counting_metrics_observer_lib",
        "//src/recorder/stream_recorder:stream_recorder_lib",
        "//src/network/ares_dns_resolver:ares_dns_resolver_lib",
        "//test/mock_satellite:mock_satellite_lib",
        "//test:ports_lib",
        "//test:string_logger_sink_lib",
        "//test:utility_lib",
    ],
)

lightstep_catch_test(
    name = "stream_recorder_fork_test",
    srcs = [
        "stream_recorder_fork_test.cpp",
    ],
    deps = [
        "//:tracer_lib",
        "//test/mock_satellite:mock_satellite_lib",
        "//test:ports_lib",
        "//test:utility_lib",
    ],
)

lightstep_catch_test(
    name = "utility_test",
    srcs = [
        "utility_test.cpp",
    ],
    deps = [
        "//src/recorder/stream_recorder:utility_lib",
        "//lightstep-tracer-common:collector_proto_cc",
    ],
)

lightstep_catch_test(
    name = "satellite_endpoint_manager_test",
    srcs = [
        "satellite_endpoint_manager_test.cpp",
    ],
    deps = [
        "//src/recorder/stream_recorder:satellite_endpoint_manager_lib",
        "//src/network/ares_dns_resolver:ares_dns_resolver_lib",
        "//test/mock_dns_server:mock_dns_server_lib",
        "//test:ports_lib",
    ],
)

lightstep_catch_test(
    name = "satellite_dns_resolution_manager_test",
    srcs = [
        "satellite_dns_resolution_manager_test.cpp",
    ],
    deps = [
        "//src/recorder/stream_recorder:satellite_dns_resolution_manager_lib",
        "//src/network/ares_dns_resolver:ares_dns_resolver_lib",
        "//test/mock_dns_server:mock_dns_server_lib",
        "//test:ports_lib",
    ],
)

lightstep_catch_test(
    name = "span_stream_test",
    srcs = [
        "span_stream_test.cpp",
    ],
    deps = [
        "//src/recorder/stream_recorder:span_stream_lib",
        "//test:utility_lib",
    ],
)

lightstep_catch_test(
    name = "connection_stream_test",
    srcs = [
        "connection_stream_test.cpp",
    ],
    deps = [
        "//src/recorder/stream_recorder:connection_stream_lib",
        "//src/recorder/serialization:report_request_header_lib",
        "//test:number_simulation_lib",
        "//test:utility_lib",
    ],
)

lightstep_catch_test(
    name = "host_header_test",
    srcs = [
        "host_header_test.cpp",
    ],
    deps = [
        "//src/recorder/stream_recorder:host_header_lib",
    ],
)

lightstep_catch_test(
    name = "status_line_parser_test",
    srcs = [
        "status_line_parser_test.cpp",
    ],
    deps = [
        "//src/recorder/stream_recorder:status_line_parser_lib",
    ],
)
