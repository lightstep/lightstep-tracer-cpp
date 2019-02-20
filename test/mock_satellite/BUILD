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
