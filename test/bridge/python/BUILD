load("@python_pip_deps//:requirements.bzl", "requirement") 
load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_cc_library",
    "lightstep_package",
)

lightstep_package()

py_test(
    name = "tracer_test",
    srcs = [
        "tracer_test.py",
    ],
    deps = [
        requirement("opentracing"),
    ],
    data = [
        "//bridge/python/binary/py3:lightstep_streaming.so",
    ],
)
