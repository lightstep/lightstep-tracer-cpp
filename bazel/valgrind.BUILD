load("@rules_foreign_cc//tools/build_defs:configure.bzl", "configure_make")

filegroup(
    name = "srcs", 
    srcs = glob(["**"]), 
)

configure_make(
    name = "valgrind",
    configure_options = [
      "CFLAGS='-fno-stack-protector'",
    ],
    lib_source = ":srcs",
    binaries = [
      "valgrind",
    ],
    visibility = ["//visibility:public"],
)

filegroup(
    name = "valgrind_filegroup",
    srcs = [":valgrind"],
    output_group = "valgrind",
    visibility = ["//visibility:public"],
)
