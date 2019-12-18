package(default_visibility = ["//visibility:public"])

load("@python_pip_deps//:requirements.bzl", "requirement") 

config_setting(
    name = "asan_build",
    values = {"define": "LIGHTSTEP_CONFIG_ASAN=1"},
)

config_setting(
    name = "tsan_build",
    values = {"define": "LIGHTSTEP_CONFIG_TSAN=1"},
)

config_setting(
    name = "portable_glibc_build",
    values = {"define": "LIGHTSTEP_PORTABLE_GLIBC=1"},
)

# Uses method described in this stackoverflow post
# https://stackoverflow.com/a/5977518/4447365
# to force linking to older glibc symbols
cc_library(
    name = "glibc_version_lib",
    srcs = [
        "glibc_version.cpp",
    ],
    linkopts = [
        "-lrt",
        "-Wl,--wrap=memcpy",
        "-Wl,--wrap=clock_gettime",
        # Note grpc already uses this wrapping technique with memcpy, so we add this flag to prevent
        # a multiple definition error
        "-Wl,--allow-multiple-definition", 
    ],
)

py_binary(
    name = "run_gprof2dot",
    srcs = [
        "run_gprof2dot.py",
    ],
    deps = [
        requirement("gprof2dot"),
    ],
)

sh_binary(
    name = "profile_benchmark",
    srcs = [
        "profile_benchmark.sh",
    ],
)
