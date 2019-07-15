workspace(name = "com_lightstep_tracer_cpp")

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "new_git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

git_repository(
    name = "rules_foreign_cc",
    remote = "https://github.com/bazelbuild/rules_foreign_cc",
    commit = "0b8356f1999d370024fc7afd924c87cb9ce77965",
)

load("@rules_foreign_cc//:workspace_definitions.bzl", "rules_foreign_cc_dependencies")

rules_foreign_cc_dependencies([
])

# git_repository(
#     name = "io_opentracing_cpp",
#     remote = "https://github.com/opentracing/opentracing-cpp",
#     commit = "ac50154a7713877f877981c33c3375003b6ebfe1",
# )

# this repo is a custom branch that was created starting with commit
# ac50154a7713877f877981c33c3375003b6ebfe1
local_repository(
    name = "io_opentracing_cpp",
    path = "C:/Users/isaac/opentracing-cpp",
)

git_repository(
    name = "com_github_googleapis_googleapis",
    remote = "https://github.com/googleapis/googleapis",
    commit = "41d72d444fbe445f4da89e13be02078734fb7875",
)

http_archive(
    name = "com_github_libevent_libevent",
    urls = [
    # bump the version !! was 8 --> 10 because of this
    # https://github.com/libevent/libevent/issues/607
        "https://github.com/libevent/libevent/archive/release-2.1.10-stable.zip"
    ],
    # sha256 = "70158101eab7ed44fd9cc34e7f247b3cae91a8e4490745d9d6eb7edc184e4d96",
    sha256 = "6353444dc9c34086a71cef7f63f4309b39df937c5da233deca290f02ccaf46a9",
    strip_prefix = "libevent-release-2.1.10-stable",
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

# original: 4c2226458203a9653ae722245cc27e8b07c383f7

# this is the one that fixed my problem, references v1.20.1 gRPC
# new: de317930548a33025a3acf430bf67e21a676084f

# current: 78d64b7317a332ee884ad7fcd0506d78f2a402cb

git_repository(
    name = "build_stack_rules_proto",
    remote = "https://github.com/stackb/rules_proto",
    commit = "4c2226458203a9653ae722245cc27e8b07c383f7",
)

# original: e97c9457e2f4e6733873ea2975d3b90432fdfdc1
# new: 7741e806a213cba63c96234f16d712a8aa101a49 # v1.20.1

# git_repository(
#    name = "com_github_grpc_grpc",
#    remote = "https://github.com/grpc/grpc",
#    commit = "e97c9457e2f4e6733873ea2975d3b90432fdfdc1",
# )
#
# load("@build_stack_rules_proto//cpp:deps.bzl", "cpp_grpc_compile")
#
# cpp_grpc_compile()
#
# load("@com_github_grpc_grpc//bazel:grpc_deps.bzl", "grpc_deps")
#
# grpc_deps()

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

go_repository(
    name = "com_github_golang_protobuf",
    importpath = "github.com/golang/protobuf",
    tag = "v1.3.0",
)
