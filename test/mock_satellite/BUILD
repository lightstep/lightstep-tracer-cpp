load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_package",
    "lightstep_cc_library",
    "lightstep_catch_test",
)

load("@io_bazel_rules_go//go:def.bzl", "go_binary", "go_library", "go_test")
load("@io_bazel_rules_go//proto:def.bzl", "go_proto_library")

lightstep_package()

go_proto_library(
    name = "collector_proto_go",
    proto = "//lightstep-tracer-common:collector_proto",
    importpath = "github.com/lightstep/lightstep-tracer-cpp/lightstep-tracer-common",
    deps = [
        "@com_github_googleapis_googleapis//google/api:annotations_go_proto",
    ],
)

proto_library(
    name = "mock_satellite_proto",
    srcs = [
        "mock_satellite.proto",
    ],
    deps = [
        "//lightstep-tracer-common:collector_proto",
    ],
)

cc_proto_library(
    name = "mock_satellite_proto_cc",
    deps = [":mock_satellite_proto"],
    visibility = ["//visibility:public"],
)

go_proto_library(
    name = "mock_satellite_proto_go",
    proto = ":mock_satellite_proto",
    importpath = "github.com/lightstep/lightstep-tracer-cpp/test/mock_satellite",
    deps = [
        ":collector_proto_go",
    ],
)

go_binary(
	name = "mock_satellite",
	srcs = [
		"main.go",
    "proto.go",
    "satellitehandler.go",
    "reportprocessor.go",
	],
  out = "mock_satellite",
  deps = [
      ":collector_proto_go",
      ":mock_satellite_proto_go",
      "@com_github_golang_protobuf//proto:go_default_library",
      "@com_github_golang_protobuf//ptypes:go_default_library",
      "@com_github_golang_protobuf//ptypes/timestamp:go_default_library",
  ],
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
        "//test:http_connection_lib",
        "//src/network:socket_lib",
        ":mock_satellite_proto_cc",
    ],
    data = [
        ":mock_satellite",
    ],
)

lightstep_catch_test(
    name = "mock_satellite_test",
    srcs = [
        "mock_satellite_test.cpp",
    ],
    deps = [
        ":mock_satellite_lib",
        "//test:http_connection_lib",
        "//test:ports_lib",
    ],
)
