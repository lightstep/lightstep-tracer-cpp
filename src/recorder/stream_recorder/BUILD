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
        "//src/common:chunk_circular_buffer_lib",
        "//src/network:event_lib",
        "//src/network:timer_event_lib",
        "//src/recorder:stream_recorder_interface",
        ":stream_recorder_options_lib",
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
    name = "satellite_connection_lib",
    private_hdrs = [
        "satellite_connection.h",
        "satellite_connection_manager.h",
    ],
    srcs = [
        "satellite_connection.cpp",
        "satellite_connection_manager.cpp",
    ],
    deps = [
        ":satellite_endpoint_manager_lib",
        "//src/common:chunk_circular_buffer_lib",
        "//src/common:noncopyable_lib",
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
)
