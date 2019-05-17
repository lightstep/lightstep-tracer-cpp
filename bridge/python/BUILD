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
        "-DPYTHON_BRIDGE_TRACER_MODULE=\"lightstep\"",
        "-Wno-missing-field-initializers",
    ],
    # Work around to https://github.com/bazelbuild/rules_foreign_cc/issues/244#issuecomment-493227536
    linkopts = [
        "-Wl,--allow-multiple-definition",
    ],
    deps = [
      "//:tracer_lib",
    ],
    external_deps = [
      "@com_github_lightstep_python_bridge_tracer//src/lib:bridge_tracer_lib",
      "@com_github_lightstep_python_bridge_tracer//3rd_party:cpython_header_only",
    ],
    visibility = [
        "//visibility:public",
    ],
)

# py_runtime(
#     name = "python-3.6.0",
#     interpreter_path = "/usr/bin/python3",
# )
