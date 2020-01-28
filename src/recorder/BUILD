load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_cc_library",
    "lightstep_package",
)

lightstep_package()

lightstep_cc_library(
    name = "grpc_transporter_interface",
    private_hdrs = [
        "grpc_transporter.h",
    ],
    deps = [
        "//include/lightstep:tracer_interface",
    ],
)

lightstep_cc_library(
    name = "no_grpc_transporter_lib",
    srcs = [
        "no_grpc_transporter.cpp",
    ],
    deps = [
        "//include/lightstep:tracer_interface",
        "//src/common:logger_lib",
        ":grpc_transporter_interface",
        "//:config_lib",
    ],
)

lightstep_cc_library(
    name = "recorder_interface",
    private_hdrs = [
      "recorder.h",
    ],
    deps = [
        "//src/common:chained_stream_lib",
        "//src/common:timestamp_lib",
        "//lightstep-tracer-common:collector_proto_cc",
        "//include/lightstep:tracer_interface",
    ],
    external_deps = [
        "@com_google_protobuf//:protobuf",
    ],
)

lightstep_cc_library(
    name = "fork_aware_recorder_lib",
    private_hdrs = [
        "fork_aware_recorder.h",
    ],
    srcs = [
        "fork_aware_recorder.cpp",
    ],
    deps = [
        "//src/common/platform:fork_lib",
        ":recorder_interface",
    ],
)

lightstep_cc_library(
    name = "report_builder_lib",
    private_hdrs = [
        "report_builder.h",
    ],
    srcs = [
        "report_builder.cpp",
    ],
    deps = [
        "//lightstep-tracer-common:collector_proto_cc",
        "//include/lightstep:tracer_interface",
        "//src/common:random_lib",
        "//src/common:utility_lib",
    ],
)

lightstep_cc_library(
    name = "legacy_manual_recorder_lib",
    private_hdrs = [
        "legacy_manual_recorder.h",
    ],
    srcs = [
        "legacy_manual_recorder.cpp",
    ],
    deps = [
        "//src/common:logger_lib",
        "//src/common:noncopyable_lib",
        "//src/common:utility_lib",
        ":recorder_interface",
        ":report_builder_lib",
        ":transporter_lib",
    ],
)

lightstep_cc_library(
    name = "manual_recorder_lib",
    private_hdrs = [
        "manual_recorder.h",
    ],
    srcs = [
        "manual_recorder.cpp",
    ],
    deps = [
        "//src/common:logger_lib",
        "//src/common:noncopyable_lib",
        "//src/common:random_lib",
        "//src/common:report_request_framing_lib",
        "//src/common:utility_lib",
        "//src/common:circular_buffer_lib",
        "//src/recorder:metrics_tracker_lib",
        "//src/recorder/serialization:report_request_lib",
        "//src/recorder/serialization:report_request_header_lib",
        ":fork_aware_recorder_lib",
        ":transporter_lib",
    ],
)

lightstep_cc_library(
    name = "auto_recorder_lib",
    private_hdrs = [
        "auto_recorder.h",
    ],
    srcs = [
        "auto_recorder.cpp",
    ],
    deps = [
        "//src/common:logger_lib",
        "//src/common:noncopyable_lib",
        "//src/common:utility_lib",
        "//src/common:condition_variable_wrapper_lib",
        ":recorder_interface",
        ":report_builder_lib",
        ":transporter_lib",
    ],
)

lightstep_cc_library(
    name = "transporter_lib",
    srcs = [
        "transporter.cpp",
    ],
    deps = [
        "//include/lightstep:transporter_interface",
        "//lightstep-tracer-common:collector_proto_cc",
    ],
)

lightstep_cc_library(
    name = "stream_recorder_interface",
    private_hdrs = [
        "stream_recorder.h",
    ],
    deps = [
        "//src/common:logger_lib",
        "//src/recorder:recorder_interface",
        "//include/lightstep:tracer_interface",
    ],
)

lightstep_cc_library(
    name = "no_stream_recorder_lib",
    srcs = [
        "no_stream_recorder.cpp",
    ],
    deps = [
        ":stream_recorder_interface",
    ],
)

lightstep_cc_library(
    name = "metrics_tracker_lib",
    private_hdrs = [
        "metrics_tracker.h",
    ],
    srcs = [
        "metrics_tracker.cpp",
    ],
    deps = [
        "//include/lightstep:tracer_interface",
    ],
)
