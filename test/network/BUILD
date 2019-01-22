load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_catch_test",
    "lightstep_package",
)

lightstep_package()

lightstep_catch_test(
    name = "event_base_test",
    srcs = [
        "event_base_test.cpp",
    ],
    deps = [
        "//src/network:event_base_lib",
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
