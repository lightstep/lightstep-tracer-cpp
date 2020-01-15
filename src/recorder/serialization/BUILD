load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_cc_library",
    "lightstep_package",
)

lightstep_package()

lightstep_cc_library(
    name = "report_request_lib",
    private_hdrs = [
        "report_request.h",
    ],
    srcs = [
        "report_request.cpp",
    ],
    deps = [
        "//include/lightstep:buffer_chain_interface",
    ],
)
