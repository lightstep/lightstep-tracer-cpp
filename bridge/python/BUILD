load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_cc_binary",
    "lightstep_package",
)

lightstep_package()

lightstep_cc_binary(
    name = "lightstep_native.so",
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

genrule(
    name = "generate_wheel",
    srcs = [
        "//:gen-config/lightstep/version.h",
        "wheel/generate_record_line.py",
        "wheel/WHEEL",
        ":lightstep_native.so",
    ],
    outs = [
        "wheel.tgz",
    ],
    cmd = """
    WORK_DIR=`mktemp -d`
    WHEEL_DIR=$$WORK_DIR/wheel
    mkdir $$WHEEL_DIR
    GENERATE_RECORD_LINE=$${PWD}/$(location wheel/generate_record_line.py)
    VERSION_H=$(location //:gen-config/lightstep/version.h)
    VERSION_=`grep LIGHTSTEP_VERSION $$VERSION_H | cut -d ' ' -f3`
    VERSION=`eval echo $$VERSION_`
    WHEEL_NAME=lightstep_native-$$VERSION-cp3-abi3-linux_x86_64

    # Set up the dist-info directory
    DISTINFO_NAME=lightstep_native-$$VERSION.dist-info
    DISTINFO_DIR=$$WORK_DIR/$$DISTINFO_NAME
    mkdir $$DISTINFO_DIR
    cp $(location wheel/WHEEL) $$DISTINFO_DIR

    # Set up the source directory
    SRC_DIR=$$WORK_DIR/lightstep_native
    mkdir $$SRC_DIR
    cp $(location :lightstep_native.so) $$SRC_DIR

    # Generate the record file

    # zip up our wheel
    pushd $$WORK_DIR
    zip -r wheel/$$WHEEL_NAME.whl $$DISTINFO_NAME lightstep_native
    tar czf wheel.tgz wheel
    popd
    cp $$WORK_DIR/wheel.tgz $${PWD}/$(location :wheel.tgz)

    rm -rf $$WORK_DIR
    """,
)
