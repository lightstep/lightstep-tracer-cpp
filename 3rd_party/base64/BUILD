load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_cc_library",
    "lightstep_package",
)

lightstep_package()

lightstep_cc_library(
    name = "base64_lib",
    hdrs = [
        "include/lightstep/base64/base64.h",
    ],
    strip_include_prefix = "include",
    srcs = [
        "src/base64.cpp",
    ],
    is_3rd_party = True,
)
