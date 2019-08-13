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
    deps = [
        "//src/common/platform:network_lib",
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
        "//src/common/platform:network_lib",
        "//src/common/platform:time_lib",
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
        "//src/common/platform:time_lib",
        ":event_lib",
    ],
    external_deps = [
        "@com_github_libevent_libevent//:libevent",
    ],
)

lightstep_cc_library(
    name = "vector_write_lib",
    private_hdrs = [
        "vector_write.h",
    ],
    srcs = [
        "vector_write.cpp",
    ],
    deps = [
        "//src/common/platform:error_lib",
        "//src/common/platform:memory_lib",
        "//src/common/platform:network_lib",
        "//src/common:fragment_input_stream_lib",
    ],
)

lightstep_cc_library(
    name = "dns_resolver_interface",
    private_hdrs = [
        "dns_resolver.h",
    ],
    deps = [
        "//src/common:logger_lib",
        "//src/common:function_ref_lib",
        ":event_lib",
        ":ip_address_lib",
    ],
    external_deps = [
        "@io_opentracing_cpp//:opentracing",
    ],
)

lightstep_cc_library(
    name = "no_dns_resolver_lib",
    srcs = [
        "no_dns_resolver.cpp",
    ],
    deps = [
        ":dns_resolver_interface",
    ],
)

lightstep_cc_library(
    name = "socket_lib",
    private_hdrs = [
        "socket.h",
    ],
    srcs = [
        "socket.cpp",
    ],
    deps = [
        "//src/common/platform:error_lib",
        ":ip_address_lib",
    ],
)
