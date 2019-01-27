load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_cc_library",
    "lightstep_package",
)

lightstep_package()

# From commit a575536abe20b58c48863600f8a71e93b4052b81
lightstep_cc_library(
    name = "catch2",
    hdrs = [
        "catch.hpp",
    ],
    is_3rd_party = True,
)

lightstep_cc_library(
    name = "main_lib",
    srcs = [
        "catch_main.cpp",
    ],
    deps = [
        ":catch2",
    ],
    is_3rd_party = True,
)
