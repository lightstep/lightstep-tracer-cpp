load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_catch_test",
    "lightstep_cc_library",
    "lightstep_package",
)

lightstep_package()

lightstep_catch_test(
    name = "report_request_header_test",
    srcs = [
        "report_request_header_test.cpp",
    ],
    deps = [
        "//src/recorder/serialization:report_request_header_lib",
    ],
)

