load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_cc_library",
    "lightstep_package",
)

lightstep_package()

lightstep_cc_library(
    name = "error_lib",
    private_hdrs = [
        "error.h",
    ],
    srcs = [
        "error_unix.cpp",
    ],
)

lightstep_cc_library(
    name = "memory_lib",
    private_hdrs = [
        "memory.h",
    ],
)

lightstep_cc_library(
    name = "network_lib",
    private_hdrs = [
        "network.h",
    ],
    srcs = [
        "network_unix.cpp",
    ],
)

lightstep_cc_library(
    name = "network_environment_lib",
    private_hdrs = [
        "network_environment.h",
    ],
    srcs = [
        "network_environment_unix.cpp",
    ],
    deps = [
        ":error_lib",
    ],
)

lightstep_cc_library(
    name = "fork_lib",
    private_hdrs = [
        "fork.h",
    ],
    srcs = [
        "fork_unix.cpp",
    ],
)

lightstep_cc_library(
    name = "string_lib",
    private_hdrs = [
        "string.h",
    ],
    srcs = [
        "string_unix.cpp",
    ],
)

lightstep_cc_library(
  name = "time_lib",
  private_hdrs = [
      "time.h",
  ],
)

lightstep_cc_library(
    name = "utility_lib",
    private_hdrs = [
        "utility.h",
    ],
    srcs = [
        "utility_unix.cpp",
    ],
)
