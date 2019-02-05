load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_package",
)

load("@io_bazel_rules_go//go:def.bzl", "go_binary", "go_library", "go_test")

lightstep_package()

go_binary(
	"mock_dns_server",
	srcs = [
		"main.go",
	],
	deps = [
		"@com_github_miekg_dns//:go_default_library",
	],
)
