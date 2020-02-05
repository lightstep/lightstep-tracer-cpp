load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_catch_test",
    "lightstep_cc_library",
    "lightstep_package",
)

lightstep_package()

lightstep_cc_library(
    name = "in_memory_recorder_lib",
    private_hdrs = [
        "in_memory_recorder.h",
    ],
    srcs = [
        "in_memory_recorder.cpp",
    ],
    deps = [
        "//src/common:buffer_chain_lib",
        "//src/recorder:recorder_interface",
        "//test:utility_lib",
        "//lightstep-tracer-common:collector_proto_cc",
    ],
)

lightstep_cc_library(
    name = "legacy_in_memory_async_transporter_lib",
    private_hdrs = [
        "legacy_in_memory_async_transporter.h",
    ],
    srcs = [
        "legacy_in_memory_async_transporter.cpp",
    ],
    deps = [
        "//src/recorder:transporter_lib",
    ],

)
lightstep_cc_library(
    name = "in_memory_async_transporter_lib",
    private_hdrs = [
        "in_memory_async_transporter.h",
    ],
    srcs = [
        "in_memory_async_transporter.cpp",
    ],
    deps = [
        "//src/common:buffer_chain_lib",
        "//src/recorder:transporter_lib",
    ],
)

lightstep_cc_library(
    name = "in_memory_sync_transporter_lib",
    private_hdrs = [
        "in_memory_sync_transporter.h",
    ],
    srcs = [
        "in_memory_sync_transporter.cpp",
    ],
    deps = [
        "//src/recorder:transporter_lib",
    ],
)

lightstep_catch_test(
    name = "legacy_manual_recorder_test",
    srcs = [
        "legacy_manual_recorder_test.cpp",
    ],
    deps = [
        "//:manual_tracer_lib",
        "//src/tracer:counting_metrics_observer_lib",
        "//test:utility_lib",
        "//test:testing_condition_variable_wrapper_lib",
        ":legacy_in_memory_async_transporter_lib",
    ],
)

lightstep_catch_test(
    name = "manual_recorder_test",
    srcs = [
        "manual_recorder_test.cpp",
    ],
    deps = [
        "//:manual_tracer_lib",
        "//src/tracer:counting_metrics_observer_lib",
        "//test:utility_lib",
        "//test:testing_condition_variable_wrapper_lib",
        ":in_memory_async_transporter_lib",
    ],
)

lightstep_catch_test(
    name = "auto_recorder_test",
    srcs = [
        "auto_recorder_test.cpp",
    ],
    deps = [
        "//:manual_tracer_lib",
        ":in_memory_sync_transporter_lib",
        "//src/tracer:counting_metrics_observer_lib",
        "//test:utility_lib",
        "//test:testing_condition_variable_wrapper_lib",
    ],
)

lightstep_catch_test(
    name = "fork_aware_recorder_test",
    srcs = [
        "fork_aware_recorder_test.cpp",
    ],
    deps = [
        "//src/recorder:fork_aware_recorder_lib",
    ],
)
