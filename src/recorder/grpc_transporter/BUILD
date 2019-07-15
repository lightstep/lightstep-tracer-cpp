load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_cc_library",
    "lightstep_package",
)

load("@build_stack_rules_proto//:compile.bzl", "proto_compile")

lightstep_package()

proto_compile(
    name = "collector_proto_grpc_files",
    plugins = [
        "@build_stack_rules_proto//cpp:grpc_cpp",
    ],
    deps = [
        "//lightstep-tracer-common:collector_proto",
    ],
)

cc_library(
    name = "collector_proto_grpc",
    srcs = select({
      "//:with_grpc": ["collector_proto_grpc_files"],
      "//conditions:default": [],
    }),
    deps = select({
      "//:with_grpc" : [
        "//lightstep-tracer-common:collector_proto_cc",
        "@com_github_grpc_grpc//:grpc++",
        "@com_github_grpc_grpc//:grpc++_reflection",
    ],
      "//conditions:default": []}),

    includes = ["collector_proto_grpc_files"],
)

lightstep_cc_library(
    name = "grpc_transporter_lib",
    srcs = [
        "grpc_transporter.cpp",
    ],
    deps = [
        "//src/recorder:grpc_transporter_interface",
        "//src/common:logger_lib",
        "//include/lightstep:tracer_interface",
        ":collector_proto_grpc",
    ],
)
