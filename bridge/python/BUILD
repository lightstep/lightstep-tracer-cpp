load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_cc_binary",
    "lightstep_package",
)

lightstep_package()

lightstep_cc_binary(
    name = "lightstep.so",
    linkshared = True,
    srcs = [
        "src/module.cpp",
    ],
    copts = [
        "-Wno-missing-field-initializers",
    ],
    linkopts = [
        "-Wl,--version-script=$(location :lightstep-export-map.ld)",
    ],
    deps = [
      ":lightstep-export-map.ld",
      "//src/tracer:lightstep_tracer_factory_lib",
      "//src/tracer:tracer_lib",
      "//src/tracer:binary_carrier_lib",
      "//src/tracer:no_default_ssl_roots_pem_lib",
      "//src/recorder:transporter_lib",
      "//src/recorder:no_grpc_transporter_lib",
      "//src/recorder/stream_recorder:stream_recorder_lib",
      "//src/network/ares_dns_resolver:ares_dns_resolver_lib",
    ],
    external_deps = [
      "@com_github_lightstep_python_bridge_tracer//:bridge_tracer_lib",
    ],
    visibility = [
        "//visibility:public",
    ],
)
