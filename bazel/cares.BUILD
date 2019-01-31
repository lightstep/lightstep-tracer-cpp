package(
    default_visibility = ["//visibility:public"],
)

licenses(["notice"])  # MIT

exports_files(["LICENSE.md"])

include_files = [
  "cares/include/ares.h",
  "cares/include/ares_build.h",
  "cares/include/ares_dns.h",
  "cares/include/ares_rules.h",
  "cares/include/ares_version.h",
]

lib_files = [
  "cares/lib/libcares.a",
]


genrule(
    name = "cares-srcs",
    outs = include_files + lib_files,
    srcs = glob(["**"]) + [
      "@local_config_cc//:toolchain",
    ],
    cmd = """
      set -e
      export INSTALL_DIR=$$PWD/$(@D)/cares
      export TMP_DIR=$$(mktemp -d -t cares.XXXXXX)
      cp -R $$(dirname $(location :README.md))/* $$TMP_DIR
      cd $$TMP_DIR
      cmake \
        -DCMAKE_INSTALL_PREFIX="$$INSTALL_DIR" \
        -DCARES_STATIC=on \
        -DCARES_SHARED=off \
        -DCARES_STATIC_PIC=on \
        -DCARES_BUILD_TESTS=off \
        -DCARES_BUILD_TOOLS=off \
        -DCMAKE_BUILD_TYPE=Release \
        .
      make
      make install
      rm -rf $$TMP_DIR
    """,
)

cc_library(
    name = "ares",
    srcs = [
        "cares/lib/libcares.a",
    ],
    hdrs = include_files,
    includes = ["cares/include"],
    linkstatic = 1,
)

filegroup(
    name = "cares-files",
    srcs = include_files + lib_files,
)
