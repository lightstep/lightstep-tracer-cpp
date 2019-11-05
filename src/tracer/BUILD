load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_cc_library",
    "lightstep_package",
)

lightstep_package()

lightstep_cc_library(
    name = "baggage_flat_map_lib",
    private_hdrs = [
        "baggage_flat_map.h",
    ],
    deps = [
        "//src/common:flat_map_lib",
    ],
    external_deps = [
        "@io_opentracing_cpp//:opentracing",
    ],
)

lightstep_cc_library(
    name = "binary_carrier_lib",
    srcs = [
        "binary_carrier.cpp",
    ],
    deps = [
        "//include/lightstep:binary_carrier_interface",
        "//include/lightstep:tracer_interface",
        ":tracer_lib",
    ],
)

lightstep_cc_library(
    name = "utility_lib",
    private_hdrs = [
        "utility.h",
    ],
    deps = [
        "//src/recorder:recorder_interface",
    ],
    external_deps = [
        "@io_opentracing_cpp//:opentracing",
    ],
)

lightstep_cc_library(
    name = "dynamic_load_lib",
    srcs = [
        "dynamic_load.cpp",
    ],
    deps = [
        ":lightstep_tracer_factory_lib",
    ],
)

lightstep_cc_library(
    name = "lightstep_span_context_interface",
    private_hdrs = [
        "lightstep_span_context.h",
    ],
    srcs = [
        "lightstep_span_context.cpp",
    ],
    deps = [
        ":propagation_lib",
    ],
)

lightstep_cc_library(
    name = "tracer_impl_lib",
    private_hdrs = [
        "tracer_impl.h",
        "span.h",
    ],
    srcs = [
        "tracer_impl.cpp",
        "span.cpp",
    ],
    deps = [
        "//src/common:logger_lib",
        "//src/common:utility_lib",
        "//src/common:random_lib",
        "//src/common:serialization_chain_lib",
        "//src/common:spin_lock_mutex_lib",
        "//src/recorder:recorder_interface",
        "//src/tracer/legacy:legacy_immutable_span_context_lib",
        ":baggage_flat_map_lib",
        ":lightstep_span_context_interface",
        ":propagation_lib",
        ":serialization_lib",
        ":utility_lib",
        ":tag_lib",
    ],
)


lightstep_cc_library(
    name = "lightstep_tracer_factory_lib",
    private_hdrs = [
        "lightstep_tracer_factory.h",
    ],
    srcs = [
        "lightstep_tracer_factory.cpp",
    ],
    deps = [
        "//lightstep-tracer-configuration:tracer_configuration_proto_cc",
        "//include/lightstep:tracer_interface",
    ],
    external_deps = [
        "@com_google_protobuf//:protobuf",
    ],
)

lightstep_cc_library(
    name = "tracer_lib",
    srcs = [
        "tracer.cpp",
    ],
    deps = [
        "//src/common/platform:utility_lib",
        "//src/recorder:auto_recorder_lib",
        "//src/recorder:grpc_transporter_interface",
        "//src/recorder:manual_recorder_lib",
        "//src/recorder:stream_recorder_interface",
        "//src/tracer/legacy:legacy_tracer_impl_lib",
        "//lightstep-tracer-common:collector_proto_cc",
        "//:config_lib",
        ":tracer_impl_lib",
    ],
    external_deps = [
        "@io_opentracing_cpp//:opentracing",
    ],
)

lightstep_cc_library(
    name = "tag_lib",
    private_hdrs = [
        "tag.h",
    ],
    srcs = [
        "tag.cpp",
    ],
    external_deps = [
        "@io_opentracing_cpp//:opentracing",
    ],
)

lightstep_cc_library(
    name = "no_default_ssl_roots_pem_lib",
    srcs = [
      "no_default_ssl_roots_pem.cpp",
    ],
)

lightstep_cc_library(
    name = "serialization_lib",
    private_hdrs = [
        "serialization.h",
    ],
    srcs = [
        "serialization.cpp",
    ],
    deps = [
        "//src/common/platform:memory_lib",
        "//src/common:serialization_lib",
        "//src/common:utility_lib",
    ],
    external_deps = [
        "@io_opentracing_cpp//:opentracing",
        "@com_google_protobuf//:protobuf",
    ],
)

lightstep_cc_library(
    name = "propagation_lib",
    private_hdrs = [
        "propagation_options.h",
        "propagation.h",
    ],
    srcs = [
        "propagation.cpp",
        "propagation_options.cpp",
    ],
    deps = [
        "//src/common:hex_conversion_lib",
        "//src/common:utility_lib",
        "//src/common:in_memory_stream_lib",
        "//src/tracer/propagation:propagator_interface",
        "//src/tracer/propagation:baggage_propagator_lib",
        "//src/tracer/propagation:multiheader_propagator_lib",
        "//src/tracer/propagation:lightstep_propagator_lib",
        "//src/tracer/propagation:b3_propagator_lib",
        "//src/tracer/propagation:envoy_propagator_lib",
        "//src/tracer/propagation:binary_propagation_lib",
        "//3rd_party/base64:base64_lib",
        "//lightstep-tracer-common:lightstep_carrier_proto_cc",
        ":baggage_flat_map_lib",
    ],
    external_deps = [
        "@com_google_protobuf//:protobuf",
        "@io_opentracing_cpp//:opentracing",
    ],
)

lightstep_cc_library(
    name = "counting_metrics_observer_lib",
    private_hdrs = [
        "counting_metrics_observer.h",
    ],
    deps = [
        "//include/lightstep:metrics_observer_interface",
    ],
)
