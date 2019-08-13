load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_cc_library",
    "lightstep_catch_test",
    "lightstep_package",
)

lightstep_package()

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
    name = "vector_write_test",
    srcs = [
        "vector_write_test.cpp",
    ],
    deps = [
        "//src/network:socket_lib",
        "//src/network:vector_write_lib",
        "//src/common:fragment_array_input_stream_lib",
        "//test/echo_server:echo_server_lib",
        "//test:ports_lib",
        "//test:utility_lib",
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
