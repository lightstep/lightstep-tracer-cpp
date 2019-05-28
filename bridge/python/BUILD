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
    deps = [
      "//:tracer_lib",
      "//src/tracer:lightstep_tracer_factory_lib",
    ],
    external_deps = [
      "@com_github_lightstep_python_bridge_tracer//:bridge_tracer_lib",
    ],
    visibility = [
        "//visibility:public",
    ],
)
