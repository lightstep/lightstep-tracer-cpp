workspace(name = "com_lightstep_tracer_cpp")

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

git_repository(
    name = "io_opentracing_cpp",
    remote = "https://github.com/opentracing/opentracing-cpp",
    commit = "ac50154a7713877f877981c33c3375003b6ebfe1",
)

local_repository(
    name = "lightstep_vendored_googleapis",
    path = "lightstep-tracer-common/third_party/googleapis",
)

git_repository(
    name = "build_stack_rules_proto",
    remote = "https://github.com/stackb/rules_proto",
    commit = "4c2226458203a9653ae722245cc27e8b07c383f7",
)

git_repository(
    name = "com_github_grpc_grpc",
    remote = "https://github.com/grpc/grpc",
    commit = "e97c9457e2f4e6733873ea2975d3b90432fdfdc1",
)

load("@build_stack_rules_proto//cpp:deps.bzl", "cpp_grpc_compile")

cpp_grpc_compile()

load("@com_github_grpc_grpc//bazel:grpc_deps.bzl", "grpc_deps")

grpc_deps()
