load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_cc_library",
    "lightstep_package",
)

lightstep_package()

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
    ],
    external_deps = [
        "@io_opentracing_cpp//:opentracing",
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
    name = "counting_metrics_observer_lib",
    private_hdrs = [
        "counting_metrics_observer.h",
    ],
    deps = [
        "//include/lightstep:metrics_observer_interface",
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
