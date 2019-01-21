load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_cc_library",
    "lightstep_package",
)

lightstep_package()

lightstep_cc_library(
    name = "grpc_transporter_interface",
    hdrs = [
        "grpc_transporter.h",
    ],
)

# lightstep_cc_library(
#     name = "grpc_transporter_lib",
#     hdrs = [
#         "grpc_transporter.h",
#     ],
#     srcs = [
#         "grpc_transporter.cpp",
#     ],
#     deps = [
#         "//include/lightstep:tracer_interface",
#         "//src/common:logger_lib",
#         "//:config_lib",
#     ],
# )

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
    hdrs = [
      "recorder.h",
    ],
    deps = [
        "//lightstep-tracer-common:collector_proto_cc",
    ],
)

lightstep_cc_library(
    name = "report_builder_lib",
    hdrs = [
        "report_builder.h",
    ],
    srcs = [
        "report_builder.cpp",
    ],
    deps = [
        "//lightstep-tracer-common:collector_proto_cc",
        "//include/lightstep:tracer_interface",
        "//src/common:utility_lib",
    ],
)

lightstep_cc_library(
    name = "manual_recorder_lib",
    hdrs = [
        "manual_recorder.h",
    ],
    srcs = [
        "manual_recorder.cpp",
    ],
    deps = [
        "//src/common:logger_lib",
        ":recorder_interface",
        ":report_builder_lib",
        ":transporter_lib",
    ],
)

lightstep_cc_library(
    name = "auto_recorder_lib",
    hdrs = [
        "auto_recorder.h",
    ],
    srcs = [
        "auto_recorder.cpp",
    ],
    deps = [
        "//src/common:logger_lib",
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
