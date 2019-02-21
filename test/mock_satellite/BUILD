load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_package",
    "lightstep_cc_library",
)

load("@io_bazel_rules_go//go:def.bzl", "go_binary", "go_library", "go_test")

lightstep_package()

go_binary(
	name = "mock_satellite",
	srcs = [
		"main.go",
	],
  out = "mock_satellite",
)

lightstep_cc_library(
    name = "mock_satellite_lib",
    private_hdrs = [
        "mock_satellite_handle.h",
    ],
    srcs = [
        "mock_satellite_handle.cpp",
    ],
    deps = [
        "//test:child_process_handle_lib",
        "//test:utility_lib",
        "//src/network:socket_lib",
    ],
    data = [
        ":mock_satellite",
    ],
)
