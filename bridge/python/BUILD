load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_portable_cc_binary",
    "lightstep_cc_library",
    "lightstep_python_wheel",
    "lightstep_package",
)

lightstep_package()

lightstep_cc_library(
    name = "python_tracer_lib",
    deps = [
      "//src/tracer:lightstep_tracer_factory_lib",
      "//src/tracer:tracer_lib",
      "//src/tracer:binary_carrier_lib",
      "//src/tracer:no_default_ssl_roots_pem_lib",
      "//src/recorder:transporter_lib",
      "//src/recorder:no_grpc_transporter_lib",
      "//src/recorder/stream_recorder:stream_recorder_lib",
      "//src/network/ares_dns_resolver:ares_dns_resolver_lib",
    ],
)

lightstep_portable_cc_binary(
    name = "lightstep_native.so",
    linkshared = True,
    srcs = [
        "//bridge/python/src:module.cpp",
        # "src/module.cpp",
    ],
    copts = [
        "-Wno-missing-field-initializers",
    ],
    linkopts = [
        "-Wl,--version-script=$(location :lightstep-export-map.ld)",
    ],
    deps = [
      ":lightstep-export-map.ld",
      ":python_tracer_lib",
    ],
    external_deps = [
      "@com_github_lightstep_python_bridge_tracer//:bridge_tracer_lib_py3",
    ],
    visibility = [
        "//visibility:public",
    ],
)

filegroup(
    name = "wheel_files",
    srcs = glob(["wheel/*"]),
)

lightstep_python_wheel(
  python_tag = "cp32",
  abi_tag = "abi3",
  binary = "//bridge/python/binary/py3:lightstep_native.so",
)

lightstep_python_wheel(
  python_tag = "cp27",
  abi_tag = "cp27m",
  binary = "//bridge/python/binary/py27m:lightstep_native.so",
)

lightstep_python_wheel(
  python_tag = "cp27",
  abi_tag = "cp27mu",
  binary = "//bridge/python/binary/py27mu:lightstep_native.so",
)

genrule(
    name = "generate_wheels2",
    srcs = [
        ":wheel-cp32-abi3.tgz",
        ":wheel-cp27-cp27m.tgz",
        ":wheel-cp27-cp27mu.tgz",
    ],
    outs = [
        "wheel2.tgz",
    ],
    cmd = """
    WORK_DIR=`mktemp -d`
    cp $(location :wheel-cp32-abi3.tgz) $$WORK_DIR
    cp $(location :wheel-cp27-cp27m.tgz) $$WORK_DIR
    cp $(location :wheel-cp27-cp27mu.tgz) $$WORK_DIR
    pushd $$WORK_DIR    
    for file in *.tgz; do
      tar zxf $$file
    done
    tar czf wheel2.tgz wheel
    popd
    cp $$WORK_DIR/wheel2.tgz $${PWD}/$(location :wheel2.tgz)
    rm -rf $$WORK_DIR
    """,
)

# Generates a wheel format suitable for uploading to Pypi
# Follows PEP 491 (https://www.python.org/dev/peps/pep-0491/)
genrule(
    name = "generate_wheel",
    srcs = [
        "//:gen-config/lightstep/version.h",
        "//:LICENSE",
        "//3rd_party/base64:LICENSE",
        "//3rd_party/randutils:LICENSE",
        ":lightstep_native.so",
        ":wheel_files",
    ],
    outs = [
        "wheel.tgz",
    ],
    cmd = """
    for file in $(locations :wheel_files); do
      WHEEL_FILES=$${PWD}/`dirname $$file`
    done
    WORK_DIR=`mktemp -d`
    WHEEL_DIR=$$WORK_DIR/wheel
    mkdir $$WHEEL_DIR
    GENERATE_RECORD=$$WHEEL_FILES/generate_record.py
    VERSION_H=$(location //:gen-config/lightstep/version.h)
    VERSION_=`grep LIGHTSTEP_VERSION $$VERSION_H | cut -d ' ' -f3`
    export VERSION=`eval echo $$VERSION_`
    WHEEL_NAME=lightstep_native-$$VERSION-cp32-abi3-manylinux1_x86_64

    # Set up the dist-info directory
    DISTINFO_NAME=lightstep_native-$$VERSION.dist-info
    DISTINFO_DIR=$$WORK_DIR/$$DISTINFO_NAME
    mkdir $$DISTINFO_DIR
    cp $$WHEEL_FILES/WHEEL $$DISTINFO_DIR
    cat $$WHEEL_FILES/METADATA.in | envsubst > $$DISTINFO_DIR/METADATA
    echo lightstep_native > $$DISTINFO_DIR/top_level.txt

    # Set up the source directory
    SRC_DIR=$$WORK_DIR/lightstep_native
    mkdir $$SRC_DIR
    cp $$WHEEL_FILES/__init__.py $$SRC_DIR
    cp $(location :lightstep_native.so) $$SRC_DIR
    cp $(location //:LICENSE) $$SRC_DIR
    mkdir -p $$SRC_DIR/3rd_party/base64
    cp $(location //3rd_party/base64:LICENSE) $$SRC_DIR/3rd_party/base64
    mkdir -p $$SRC_DIR/3rd_party/randutils
    cp $(location //3rd_party/randutils:LICENSE) $$SRC_DIR/3rd_party/randutils

    # Generate the record file
    python $$GENERATE_RECORD $$WORK_DIR > $$DISTINFO_DIR/RECORD
    echo "$$DISTINFO_NAME/RECORD,," >> $$DISTINFO_DIR/RECORD
    
    # zip up our wheel
    pushd $$WORK_DIR
    zip -r wheel/$$WHEEL_NAME.whl $$DISTINFO_NAME lightstep_native
    tar czf wheel.tgz wheel
    popd
    cp $$WORK_DIR/wheel.tgz $${PWD}/$(location :wheel.tgz)

    rm -rf $$WORK_DIR
    """,
)
