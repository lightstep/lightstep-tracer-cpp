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

lightstep_catch_test(
    name = "embedded_metrics_message_test",
    srcs = [
        "embedded_metrics_message_test.cpp",
    ],
    deps = [
        "//src/recorder/serialization:embedded_metrics_message_lib",
    ],
)

lightstep_catch_test(
    name = "report_request_test",
    srcs = [
        "report_request_test.cpp",
    ],
    deps = [
        "//src/common:report_request_framing_lib",
        "//src/recorder/serialization:report_request_lib",
        "//src/recorder/serialization:report_request_header_lib",
    ],
)
