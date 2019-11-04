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
        "@io_opentracing_cpp//:opentracing",
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
        "//src/tracer:baggage_flat_map_lib",
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
