workspace(name = "com_lightstep_tracer_cpp")

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "new_git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# In order to statically link in the standard c++ library to produce portable binaries, we need
# to wrap the compiler command to work around
# See https://github.com/bazelbuild/bazel/issues/4644
load("//bazel:cc_configure.bzl", "cc_configure")
cc_configure()

git_repository(
    name = "io_opentracing_cpp",
    remote = "https://github.com/opentracing/opentracing-cpp",
    commit = "ac50154a7713877f877981c33c3375003b6ebfe1",
)

git_repository(
    name = "com_google_protobuf",
    remote = "https://github.com/protocolbuffers/protobuf.git",
    commit = "8e5ea65953f3c47e01bca360ecf3abdf2c8b1c33",
)

git_repository(
    name = "com_github_googleapis_googleapis",
    remote = "https://github.com/googleapis/googleapis",
    commit = "41d72d444fbe445f4da89e13be02078734fb7875",
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
    commit = "75475f090875e737ad6909a6057c59577f0c79b1",
)

load("@build_stack_rules_proto//cpp:deps.bzl", "cpp_grpc_compile")

cpp_grpc_compile()

load("@com_github_grpc_grpc//bazel:grpc_deps.bzl", "grpc_deps")

grpc_deps()

#######################################
# Python bridge
#######################################

git_repository(
    name = "com_github_lightstep_python_bridge_tracer",
    remote = "https://github.com/lightstep/python-bridge-tracer.git",
    commit = "e41e080324c14a4a0a62833c7ad8c522b5e60494",
) 

http_archive(
    name = "com_github_python_cpython",
    build_file = "@com_github_lightstep_python_bridge_tracer//bazel:cpython.BUILD",
    strip_prefix = "cpython-3.7.3",
    urls = [
        "https://github.com/python/cpython/archive/v3.7.3.tar.gz",
    ],
)


new_git_repository(
    name = "vendored_pyconfig",
    remote = "https://github.com/rnburn/python-bridge-tracer.git",
    commit = "9c14ba0add5db148f5736a60b15e96b7afc3ba19",
    build_file = "@com_github_lightstep_python_bridge_tracer//bazel:vendored_pyconfig.BUILD",
) 

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

# Only needed for PIP support:
git_repository(
    name = "io_bazel_rules_python",
    remote = "https://github.com/bazelbuild/rules_python.git",
    commit = "965d4b4a63e6462204ae671d7c3f02b25da37941",
)

load("@io_bazel_rules_python//python:pip.bzl", "pip_repositories")

pip_repositories()

load("@io_bazel_rules_python//python:pip.bzl", "pip_import")

pip_import(
    name = "python_pip_deps",
    requirements = "//bridge/python:requirements.txt",
)

load("@python_pip_deps//:requirements.bzl", "pip_install")
pip_install()
