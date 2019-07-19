load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_cc_library",
    "lightstep_package",
)

lightstep_package()

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
