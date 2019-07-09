load("@rules_foreign_cc//tools/build_defs:cmake.bzl", "cmake_external")


filegroup(
    name = "srcs",
    srcs = glob(["**"]),
)

cmake_external(
    name = "ares",
    cache_entries = {
      "CMAKE_BUILD_TYPE": "Release",
      "CARES_SHARED": "off",
      "CARES_STATIC": "on",
      "CARES_STATIC_PIC": "on",
      "CARES_BUILD_TESTS": "off",
      "CARES_BUILD_TOOLS": "off",
    },
    lib_source = ":srcs",
    static_libraries = ["cares.lib"], # used to be libcares.a
    visibility = ["//visibility:public"],

    ## required for Windows ##
    generate_crosstool_file = True,
    cmake_options = ["-G \"NMake Makefiles\""], # use an NMake generator
    make_commands = [ # NMake build / install commands
        "nmake",
        "nmake install",
    ],
)
