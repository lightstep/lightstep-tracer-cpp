load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_cc_library",
    "lightstep_package",
)

lightstep_package()

lightstep_cc_library(
    name = "binary_carrier_interface",
    hdrs = [
        "binary_carrier.h",
    ],
    deps = [
        "//lightstep-tracer-common:lightstep_carrier_proto_cc",
    ],
    external_deps = [
        "@io_opentracing_cpp//:opentracing",
    ],
)

lightstep_cc_library(
    name = "buffer_chain_interface",
    hdrs = [
        "buffer_chain.h",
    ],
)

lightstep_cc_library(
    name = "transporter_interface",
    hdrs = [
        "transporter.h",
    ],
    external_deps = [
        ":buffer_chain_interface",
        "@com_google_protobuf//:protobuf",
        "@io_opentracing_cpp//:opentracing",
    ],
)

lightstep_cc_library(
    name = "metrics_observer_interface",
    hdrs = [
        "metrics_observer.h",
    ],
)

lightstep_cc_library(
    name = "tracer_interface",
    hdrs = [
        "tracer.h",
    ],
    deps = [
        ":transporter_interface",
        ":metrics_observer_interface",
    ],
    external_deps = [
        "@io_opentracing_cpp//:opentracing",
    ],
)
