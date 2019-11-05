load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_cc_library",
    "lightstep_package",
)

lightstep_package()

lightstep_cc_library(
    name = "propagator_interface",
    private_hdrs = [
        "propagator.h",
    ],
    external_deps = [
        "@com_google_protobuf//:protobuf",
        "@io_opentracing_cpp//:opentracing",
    ],
    deps = [
        "//src/tracer:baggage_flat_map_lib",
    ],
)

lightstep_cc_library(
    name = "multiheader_propagator_lib",
    private_hdrs = [
        "multiheader_propagator.h",
    ],
    srcs = [
        "multiheader_propagator.cpp",
    ],
    deps = [
        "//src/common:hex_conversion_lib",
        "//src/common:utility_lib",
        ":propagator_interface",
    ],
)

lightstep_cc_library(
    name = "lightstep_propagator_lib",
    private_hdrs = [
        "lightstep_propagator.h",
    ],
    srcs = [
        "lightstep_propagator.cpp",
    ],
    deps = [
        ":multiheader_propagator_lib",
    ],
)

lightstep_cc_library(
    name = "b3_propagator_lib",
    private_hdrs = [
        "b3_propagator.h",
    ],
    srcs = [
        "b3_propagator.cpp",
    ],
    deps = [
        ":lightstep_propagator_lib",
    ],
)

lightstep_cc_library(
    name = "baggage_propagator_lib",
    private_hdrs = [
        "baggage_propagator.h",
    ],
    srcs = [
        "baggage_propagator.cpp",
    ],
    deps = [
        ":propagator_interface",
    ],
)

# lightstep_cc_library(
#     name = "propagation_options_lib",
#     private_hdrs = [
#         "propagation_options.h",
#     ],
#     srcs = [
#         "propagation_options.cpp",
#     ],
#     deps = [
#         ":propagator_interface",
#     ],
# )
