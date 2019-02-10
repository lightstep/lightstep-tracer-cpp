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

http_archive(
    name = "com_github_libevent_libevent",
    urls = [
        "https://github.com/libevent/libevent/archive/release-2.1.8-stable.zip"
    ],
    sha256 = "70158101eab7ed44fd9cc34e7f247b3cae91a8e4490745d9d6eb7edc184e4d96",
    strip_prefix = "libevent-release-2.1.8-stable",
    build_file = "//bazel:libevent.BUILD",
)

http_archive(
    name = "com_github_cares_cares",
    urls = [
        "https://github.com/c-ares/c-ares/releases/download/cares-1_15_0/c-ares-1.15.0.tar.gz",
    ],
    sha256 = "6cdb97871f2930530c97deb7cf5c8fa4be5a0b02c7cea6e7c7667672a39d6852",
    strip_prefix = "c-ares-1.15.0",
    build_file = "//bazel:cares.BUILD",
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

#######################################
# Testing dependencies
#######################################

git_repository(
    name = "com_google_benchmark",
    commit = "e776aa0275e293707b6a0901e0e8d8a8a3679508",
    remote = "https://github.com/google/benchmark",
)

http_archive(
    name = "io_bazel_rules_go",
    urls = ["https://github.com/bazelbuild/rules_go/releases/download/0.17.0/rules_go-0.17.0.tar.gz"],
    sha256 = "492c3ac68ed9dcf527a07e6a1b2dcbf199c6bf8b35517951467ac32e421c06c1",
)

load("@io_bazel_rules_go//go:deps.bzl", "go_rules_dependencies", "go_register_toolchains")
go_rules_dependencies()
go_register_toolchains()

http_archive(
    name = "bazel_gazelle",
    urls = ["https://github.com/bazelbuild/bazel-gazelle/releases/download/0.16.0/bazel-gazelle-0.16.0.tar.gz"],
    sha256 = "7949fc6cc17b5b191103e97481cf8889217263acf52e00b560683413af204fcb",
)

load("@bazel_gazelle//:deps.bzl", "gazelle_dependencies", "go_repository")
gazelle_dependencies()

go_repository(
	name = "com_github_miekg_dns",
	importpath = "github.com/miekg/dns",
	tag = "v1.1.4",
)
