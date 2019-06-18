package(default_visibility = ["//visibility:public"])

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

cc_library(
    name = "glibc_version_lib",
    srcs = [
        "glibc_version.c",
    ],
    linkopts = [
        "-lrt",
        "-Wl,--wrap=memcpy",
        "-Wl,--wrap=clock_gettime",
    ],
)
