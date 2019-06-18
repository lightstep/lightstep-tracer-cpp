load("@rules_foreign_cc//tools/build_defs:cmake.bzl", "cmake_external")


filegroup(
    name = "srcs", 
    srcs = glob(["**"]), 
)

cmake_external(
    name = "libevent",
    cache_entries = {
        "BUILD_SHARED_LIBS": "OFF",
        "BUILD_STATIC_LIBS": "ON",
    },
    cmake_options = [
        "-DEVENT__DISABLE_OPENSSL:BOOL=on",
        "-DEVENT__DISABLE_REGRESS:BOOL=on",
        "-DCMAKE_POSITION_INDEPENDENT_CODE=on",
    ],
    lib_source = ":srcs",
    static_libraries = ["libevent.a"],
    visibility = ["//visibility:public"],
)
