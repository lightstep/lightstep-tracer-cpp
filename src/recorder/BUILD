load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_cc_library",
    "lightstep_package",
)

lightstep_package()

# lightstep_cc_library(
#     name = "grpc_transporter_lib",
#     hdrs = [
#         "grpc_transporter.h",
#         "//:include/lightstep/config.h",
#     ],
#     srcs = [
#         "grpc_transporter.cpp",
#     ],
#     deps = [
#         "//include/lightstep:tracer_interface",
#         "//src/common:logger_lib",
#     ],
# )

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
        "//include/lightstep:transporter_interface",
        ":recorder_interface",
        ":report_builder_lib",
    ],
)
