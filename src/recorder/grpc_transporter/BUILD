load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_cc_library",
    "lightstep_package",
)

# load("@build_stack_rules_proto//:compile.bzl", "proto_compile")
# load("@build_stack_rules_proto//cpp:cpp_grpc_library.bzl", "cpp_grpc_library")
# load("@build_stack_rules_proto//cpp:cpp_grpc_library.bzl", "cpp_grpc_library")
# load("@com_github_grpc_grpc//bazel:cc_grpc_library.bzl", "cc_grpc_library")

lightstep_package()

# proto_compile(
#     name = "collector_proto_grpc_files",
#     plugins = [
#         "@build_stack_rules_proto//cpp:grpc_cpp",
#     ],
#     deps = [
#         "//lightstep-tracer-common:collector_proto",
#     ],
# )

# cc_library(
#     name = "collector_proto_grpc",
#     srcs = [
#         "collector_proto_grpc_files",
#     ],
#     deps = [
#         "//lightstep-tracer-common:collector_proto_cc",
#         "@com_github_grpc_grpc//:grpc++",
#         "@com_github_grpc_grpc//:grpc++_reflection",
#     ],
#     includes = ["collector_proto_grpc_files"],
# )

# cpp_grpc_library(
#     name = "collector_proto_grpc",
#     deps = [
#         "//lightstep-tracer-common:collector_proto",
#     ],
# )

# cc_grpc_library(
#     name = "collector_proto_grpc",
#     srcs = [
#         "//lightstep-tracer-common:collector_proto",
#     ],
#     deps = [
#         "//lightstep-tracer-common:collector_proto_cc",
#     ],
#     grpc_only = True,
# )

lightstep_cc_library(
    name = "grpc_transporter_lib",
    srcs = [
        "grpc_transporter.cpp",
    ],
    deps = [
        "//src/recorder:grpc_transporter_interface",
        "//src/common:logger_lib",
        "//include/lightstep:tracer_interface",
        "//lightstep-tracer-common:collector_proto_grpc",
    ],
)
