load("@rules_foreign_cc//tools/build_defs:cmake.bzl", "cmake_external")


filegroup(
    name = "srcs", 
    srcs = glob(["**"]), 
)

cmake_external(
    name = "ares",
    cmake_options = [
        "-DEVENT__DISABLE_OPENSSL:BOOL=on",
        "-DEVENT__DISABLE_REGRESS:BOOL=on",
        "-DCARES_STATIC=on",
        "-DCARES_SHARED=off",
        "-DCARES_STATIC_PIC=on",
        "-DCARES_BUILD_TESTS=off",
        "-DCARES_BUILD_TOOLS=off",
    ],
    lib_source = ":srcs",
    static_libraries = ["libcares.a"],
    visibility = ["//visibility:public"],
)

