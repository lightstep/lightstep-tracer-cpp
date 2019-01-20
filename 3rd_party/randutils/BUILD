load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_cc_library",
    "lightstep_package",
)

lightstep_package()

lightstep_cc_library(
    name = "randutils_lib",
    srcs = [
        "include/lightstep/randutils.h",
    ],
    is_3rd_party = True,
    includes = [
        "include",
    ],
)
