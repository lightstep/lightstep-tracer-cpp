load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_cc_library",
    "lightstep_package",
)

lightstep_package()

lightstep_cc_library(
    name = "utility_lib",
    hdrs = [
        "utility.h",
    ],
    srcs = [
        "utility.cpp",
    ],
    deps = [
        "//lightstep-tracer-common:collector_proto_cc",
        "//src/common:utility_lib",
    ],
    external_deps = [
        "@io_opentracing_cpp//:opentracing",
    ],
)


lightstep_cc_library(
    name = "testing_condition_variable_wrapper_lib",
    hdrs = [
        "testing_condition_variable_wrapper.h",
    ],
    srcs = [
        "testing_condition_variable_wrapper.cpp",
    ],
    deps = [
        "//src/common:condition_variable_wrapper_lib",
    ],
)

lightstep_cc_library(
    name = "counting_metrics_observer_lib",
    hdrs = [
        "counting_metrics_observer.h",
    ],
    deps = [
        "//include/lightstep:metrics_observer_interface",
    ],
)
