load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_portable_cc_binary",
    "lightstep_package",
)

lightstep_package()

lightstep_portable_cc_binary(
    name = "lightstep_streaming.so",
    linkshared = True,
    srcs = [
        "//bridge/python/src:module.cpp",
    ],
    copts = [
        "-Wno-missing-field-initializers",
    ],
    linkopts = [
        "-Wl,--version-script=$(location :lightstep-export-map.ld)",
    ],
    deps = [
      ":lightstep-export-map.ld",
      "//bridge/python:python_tracer_lib",
    ],
    external_deps = [
      "@com_github_lightstep_python_bridge_tracer//:bridge_tracer_lib_py27m",
    ],
    visibility = [
        "//visibility:public",
    ],
)
