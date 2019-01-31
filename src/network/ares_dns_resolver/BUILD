load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_cc_library",
    "lightstep_package",
)

lightstep_package()

lightstep_cc_library(
    name = "ares_dns_resolver_lib",
    srcs = [
        "ares_dns_resolver.h",
        "ares_dns_resolver.cpp",
        "ares_library_handle.h",
        "ares_library_handle.cpp",
    ],
    deps = [
        "//src/network:dns_resolver_interface",
    ],
    external_deps = [
        "@com_github_cares_cares//:ares",
    ],
)
