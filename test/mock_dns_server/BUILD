load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_package",
    "lightstep_cc_library",
    "lightstep_go_binary",
)

lightstep_package()

lightstep_go_binary(
	name = "mock_dns_server",
	srcs = [
		"main.go",
	],
  out = "mock_dns_server",
	deps = [
		"@com_github_miekg_dns//:go_default_library",
	],
)

lightstep_cc_library(
    name = "mock_dns_server_lib",
    private_hdrs = [
        "mock_dns_server_handle.h",
    ],
    srcs = [
        "mock_dns_server_handle.cpp",
    ],
    deps = [
        "//test:child_process_handle_lib",
        "//test:utility_lib",
        "//test:mock_dns_resolution_callback_lib",
        "//src/network/ares_dns_resolver:ares_dns_resolver_lib",
    ],
    data = [
        ":mock_dns_server",
    ],
)
