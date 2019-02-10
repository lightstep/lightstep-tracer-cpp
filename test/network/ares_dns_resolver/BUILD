load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_catch_test",
    "lightstep_package",
)

lightstep_package()

lightstep_catch_test(
    name = "ares_dns_resolver_test",
    srcs = [
        "ares_dns_resolver_test.cpp",
    ],
    deps = [
        "//src/network/ares_dns_resolver:ares_dns_resolver_lib",
        "//test/mock_dns_server:mock_dns_server_lib",
        "//test:ports_lib",
    ],
)

