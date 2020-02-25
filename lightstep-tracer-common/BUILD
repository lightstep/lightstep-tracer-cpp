load("@rules_proto//proto:defs.bzl", "proto_library")

proto_library(
    name = "collector_proto",
    srcs = ["collector.proto"],
    deps = [
        "@com_google_googleapis//google/api:annotations_proto",
        "@com_google_protobuf//:descriptor_proto",
        "@com_google_protobuf//:timestamp_proto",
    ],
    visibility = ["//visibility:public"],
)

cc_proto_library(
    name = "collector_proto_cc",
    deps = [":collector_proto"],
    visibility = ["//visibility:public"],
)

proto_library(
    name = "lightstep_carrier_proto",
    srcs = ["lightstep_carrier.proto"],
    visibility = ["//visibility:public"],
)

cc_proto_library(
    name = "lightstep_carrier_proto_cc",
    deps = [":lightstep_carrier_proto"],
    visibility = ["//visibility:public"],
)

load("@com_github_grpc_grpc//bazel:cc_grpc_library.bzl", "cc_grpc_library")

cc_grpc_library(
    name = "collector_proto_grpc",
    srcs = [
        ":collector_proto",
    ],
    deps = [
        ":collector_proto_cc",
        "@com_github_grpc_grpc//:grpc++",
        "@com_github_grpc_grpc//:grpc++_reflection",
    ],
    grpc_only = True,
    visibility = ["//visibility:public"],
)
