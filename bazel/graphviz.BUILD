load("@rules_foreign_cc//tools/build_defs:configure.bzl", "configure_make")

filegroup(
    name = "srcs", 
    srcs = glob(["**"]), 
)

configure_make(
    name = "graphviz",
    lib_source = ":srcs",
    binaries = [
      "dot",
    ],
    visibility = ["//visibility:public"],
)

filegroup(
    name = "dot_filegroup",
    srcs = [":graphviz"],
    output_group = "dot",
    visibility = ["//visibility:public"],
)

