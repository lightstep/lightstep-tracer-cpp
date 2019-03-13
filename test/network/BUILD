load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_cc_library",
    "lightstep_catch_test",
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
        "//src/network:fixed_fragment_input_stream_lib",
    ],
)

lightstep_catch_test(
    name = "event_test",
    srcs = [
        "event_test.cpp",
    ],
    deps = [
        "//src/network:event_lib",
    ],
)

lightstep_catch_test(
    name = "event_base_test",
    srcs = [
        "event_base_test.cpp",
    ],
    deps = [
        "//src/network:event_lib",
    ],
)

lightstep_catch_test(
    name = "fixed_fragment_input_stream_test",
    srcs = [
        "fixed_fragment_input_stream_test.cpp",
    ],
    deps = [
        "//src/network:fixed_fragment_input_stream_lib",
        ":utility_lib",
    ],
)

lightstep_catch_test(
    name = "fragment_input_stream_test",
    srcs = [
        "fragment_input_stream_test.cpp",
    ],
    deps = [
        "//src/network:fragment_input_stream_lib",
        ":utility_lib",
    ],
)

lightstep_catch_test(
    name = "fragment_array_input_stream_test",
    srcs = [
        "fragment_array_input_stream_test.cpp",
    ],
    deps = [
        "//src/network:fragment_array_input_stream_lib",
        ":utility_lib",
    ],
)


lightstep_catch_test(
    name = "vector_write_test",
    srcs = [
        "vector_write_test.cpp",
    ],
    deps = [
        "//src/network:socket_lib",
        "//src/network:vector_write_lib",
        "//test/echo_server:echo_server_lib",
        "//test:ports_lib",
        "//test/network:utility_lib",
    ],
)

lightstep_catch_test(
    name = "timer_event_test",
    srcs = [
        "timer_event_test.cpp",
    ],
    deps = [
        "//src/network:timer_event_lib",
    ],
)

lightstep_catch_test(
    name = "ip_address_test",
    srcs = [
        "ip_address_test.cpp",
    ],
    deps = [
        "//src/network:ip_address_lib",
    ],
)

lightstep_catch_test(
    name = "socket_test",
    srcs = [
        "socket_test.cpp",
    ],
    deps = [
        "//src/network:socket_lib",
    ],
)

lightstep_catch_test(
    name = "no_dns_resolver_test",
    srcs = [
        "no_dns_resolver_test.cpp",
    ],
    deps = [
      "//test:mock_dns_resolution_callback_lib",
      "//src/network:no_dns_resolver_lib",
    ],
)
