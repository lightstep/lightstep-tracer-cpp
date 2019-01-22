load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_cc_library",
    "lightstep_package",
)

lightstep_package()

lightstep_cc_library(
    name = "event_base_lib",
  private_hdrs = [
      "event_base.h",
  ],
  srcs = [
      "event_base.cpp",
  ],
  deps = [
      "//src/common:utility_lib",
  ],
  external_deps = [
      "@com_github_libevent_libevent//:libevent",
  ],
)
