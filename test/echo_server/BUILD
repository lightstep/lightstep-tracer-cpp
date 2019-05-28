load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_package",
    "lightstep_cc_library",
    "lightstep_go_binary",
)

lightstep_package()

lightstep_go_binary(
	name = "echo_server",
	srcs = [
		"main.go",
	],
  out = "echo_server",
)

lightstep_cc_library(
    name = "echo_server_lib",
    private_hdrs = [
        "echo_server_handle.h",
    ],
    srcs = [
        "echo_server_handle.cpp",
    ],
    deps = [
        "//test:child_process_handle_lib",
        "//test:utility_lib",
        "//test:http_connection_lib",
    ],
    data = [
        ":echo_server",
    ],
)
