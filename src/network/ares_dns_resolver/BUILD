load(
    "//bazel:lightstep_build_system.bzl",
    "lightstep_cc_library",
    "lightstep_package",
)

lightstep_package()

# somehow missing these:
# 'external/com_github_cares_cares/ares.h'
# 'external/com_github_cares_cares/ares_version.h'
# 'external/com_github_cares_cares/ares_build.h'
# 'external/com_github_cares_cares/ares_rules.h'
lightstep_cc_library(
    name = "ares_dns_resolver_lib",
    srcs = [
        "ares_dns_resolver.h",
        "ares_dns_resolver.cpp",
        "ares_library_handle.h",
        "ares_library_handle.cpp",
    ],
    deps = [
        "//src/common:noncopyable_lib",
        "//src/network:dns_resolver_interface",
        "//src/network:event_lib",
        "//src/network:timer_event_lib",
    ],
    external_deps = [
        "@com_github_cares_cares//:ares",
    ],
    includes = ["external/com_github_cares_cares/cares/include"]
)
