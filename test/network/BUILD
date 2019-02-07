load(
    "//bazel:lightstep_build_system.bzl",
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
