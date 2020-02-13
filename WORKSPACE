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
    name = "rules_foreign_cc",
    remote = "https://github.com/bazelbuild/rules_foreign_cc",
    commit = "ed3db61a55c13da311d875460938c42ee8bbc2a5",
)

load("@rules_foreign_cc//:workspace_definitions.bzl", "rules_foreign_cc_dependencies")

rules_foreign_cc_dependencies([
])

git_repository(
    name = "io_opentracing_cpp",
    remote = "https://github.com/opentracing/opentracing-cpp",
    commit = "ac50154a7713877f877981c33c3375003b6ebfe1",
)

git_repository(
    name = "com_google_protobuf",
    remote = "https://github.com/protocolbuffers/protobuf.git",
    commit = "498de9f761bef56a032815ee44b6e6dbe0892cc4",
)

load("@com_google_protobuf//:protobuf_deps.bzl", "protobuf_deps")

protobuf_deps()

git_repository(
    name = "com_github_googleapis",
    remote = "https://github.com/googleapis/googleapis",
    commit = "e46f761cd6ec15a9e3d5ed4ff321a4bcba8e8585",
)
load("@com_github_googleapis//:repository_rules.bzl", "switched_rules_by_language")
switched_rules_by_language(
    name = "com_google_googleapis_imports",
    grpc = True,
    cc = True,
    go = True,
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
    name = "com_github_grpc_grpc",
    remote = "https://github.com/grpc/grpc",
    commit = "8cf19cfdd840731bb31665a757fd1074e58e8ce4",
)

load("@com_github_grpc_grpc//bazel:grpc_deps.bzl", "grpc_deps")

grpc_deps()

load("@com_github_grpc_grpc//bazel:grpc_extra_deps.bzl", "grpc_extra_deps")

grpc_extra_deps()

#######################################
# Python bridge
#######################################

git_repository(
    name = "com_github_lightstep_python_bridge_tracer",
    remote = "https://github.com/lightstep/python-bridge-tracer.git",
    commit = "684be53ccb7cbee568d55c242cc6fb1210be7595",
) 

http_archive(
    name = "com_github_python_cpython3",
    build_file = "@com_github_lightstep_python_bridge_tracer//bazel:cpython.BUILD",
    strip_prefix = "cpython-3.7.3",
    urls = [
        "https://github.com/python/cpython/archive/v3.7.3.tar.gz",
    ],
)

http_archive(
    name = "com_github_python_cpython27",
    build_file = "@com_github_lightstep_python_bridge_tracer//bazel:cpython.BUILD",
    strip_prefix = "cpython-2.7.16",
    urls = [
        "https://github.com/python/cpython/archive/v2.7.16.tar.gz",
    ],
)


new_git_repository(
    name = "vendored_pyconfig3",
    remote = "https://github.com/lightstep/python-bridge-tracer.git",
    commit = "02908b06a2509f27fbb910244470f3fc70a5d2a3",
    build_file = "@com_github_lightstep_python_bridge_tracer//bazel:vendored_pyconfig3.BUILD",
) 

new_git_repository(
    name = "vendored_pyconfig27m",
    remote = "https://github.com/lightstep/python-bridge-tracer.git",
    commit = "02908b06a2509f27fbb910244470f3fc70a5d2a3",
    build_file = "@com_github_lightstep_python_bridge_tracer//bazel:vendored_pyconfig27m.BUILD",
) 

new_git_repository(
    name = "vendored_pyconfig27mu",
    remote = "https://github.com/lightstep/python-bridge-tracer.git",
    commit = "02908b06a2509f27fbb910244470f3fc70a5d2a3",
    build_file = "@com_github_lightstep_python_bridge_tracer//bazel:vendored_pyconfig27mu.BUILD",
) 

#######################################
# Testing dependencies
#######################################

git_repository(
    name = "com_google_benchmark",
    commit = "e776aa0275e293707b6a0901e0e8d8a8a3679508",
    remote = "https://github.com/google/benchmark",
)

git_repository(
        name   = "com_github_gflags_gflags",
        #tag    = "v2.2.2",
        commit = "e171aa2d15ed9eb17054558e0b3a6a413bb01067",
        remote = "https://github.com/gflags/gflags.git"
)

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "io_bazel_rules_go",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/rules_go/releases/download/v0.19.10/rules_go-v0.19.10.tar.gz",
        "https://github.com/bazelbuild/rules_go/releases/download/v0.19.10/rules_go-v0.19.10.tar.gz",
    ],
    sha256 = "0a99597dd30ecfd94f64fc64717cd1b6c4bd9807918942a8501a3883990e4b1d",
)

load("@io_bazel_rules_go//go:deps.bzl", "go_rules_dependencies", "go_register_toolchains")

go_rules_dependencies()
go_register_toolchains()

http_archive(
    name = "bazel_gazelle",
    urls = ["https://github.com/bazelbuild/bazel-gazelle/releases/download/0.18.2/bazel-gazelle-0.18.2.tar.gz"],
    sha256 = "7fc87f4170011201b1690326e8c16c5d802836e3a0d617d8f75c3af2b23180c4",
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

http_archive(
    name = "org_valgrind",
    urls = [
        "https://sourceware.org/pub/valgrind/valgrind-3.15.0.tar.bz2",
    ],
    sha256 = "417c7a9da8f60dd05698b3a7bc6002e4ef996f14c13f0ff96679a16873e78ab1",
    strip_prefix = "valgrind-3.15.0",
    build_file = "//bazel:valgrind.BUILD",
)

http_archive(
    name = "org_graphviz",
    urls = [
        "https://graphviz.gitlab.io/pub/graphviz/stable/SOURCES/graphviz.tar.gz",
    ],
    sha256 = "ca5218fade0204d59947126c38439f432853543b0818d9d728c589dfe7f3a421",
    strip_prefix = "graphviz-2.40.1",
    build_file = "//bazel:graphviz.BUILD",
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
