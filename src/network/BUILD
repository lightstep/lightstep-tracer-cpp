load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_cc_library",
    "lightstep_package",
)

lightstep_package()

lightstep_cc_library(
    name = "ip_address_lib",
    private_hdrs = [
        "ip_address.h",
    ],
    srcs = [
        "ip_address.cpp",
    ],
)

lightstep_cc_library(
    name = "event_lib",
    private_hdrs = [
      "event.h",
      "event_base.h",
    ],
    srcs = [
        "event.cpp",
        "event_base.cpp",
    ],
    deps = [
        "//src/common:utility_lib",
    ],
    external_deps = [
        "@com_github_libevent_libevent//:libevent",
    ],
)

lightstep_cc_library(
    name = "timer_event_lib",
    private_hdrs = [
        "timer_event.h",
    ],
    srcs = [
        "timer_event.cpp",
    ],
    deps = [
        "//src/common:utility_lib",
        ":event_lib",
    ],
    external_deps = [
        "@com_github_libevent_libevent//:libevent",
    ],
)

lightstep_cc_library(
    name = "dns_resolver_interface",
    private_hdrs = [
        "dns_resolver.h",
    ],
    deps = [
        "//src/common:logger_lib",
        ":event_lib",
        ":ip_address_lib",
    ],
    external_deps = [
        "@io_opentracing_cpp//:opentracing",
    ],
)
