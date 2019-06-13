load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_cc_library",
    "lightstep_package",
)

lightstep_package()

exports_files(["LICENSE"])

lightstep_cc_library(
    name = "base64_lib",
    private_hdrs = [
        "include/lightstep/base64/base64.h",
    ],
    srcs = [
        "src/base64.cpp",
    ],
    is_3rd_party = True,
)
