load("@io_bazel_rules_go//go:def.bzl", "go_binary", "go_test")

def lightstep_package():
    native.package(default_visibility = ["//visibility:public"])

# Taken from https://github.com/bazelbuild/bazel/issues/2670#issuecomment-369674735
# with modifications.
def lightstep_private_include_copts(includes, is_system=False):
    copts = []
    prefix = ''

    # convert "@" to "external/" unless in the main workspace
    repo_name = native.repository_name()
    if repo_name != '@':
        prefix = 'external/{}/'.format(repo_name[1:])

    inc_flag = "-isystem " if is_system else "-I"

    for inc in includes:
        copts.append("{}{}{}".format(inc_flag, prefix, inc))
        copts.append("{}$(GENDIR){}/{}".format(inc_flag, prefix, inc))

    return copts

def lightstep_include_copts():
  return lightstep_private_include_copts([
      "include",
      "src",
      "test",
  ]) + lightstep_private_include_copts([
    "3rd_party/randutils/include", 
    "3rd_party/base64/include", 
  ], is_system=True)

def lightstep_copts(is_3rd_party=False):
  if is_3rd_party:
    return [
      "-std=c++11",
    ]
  return [
      "-Wall",
      "-Wextra",
      "-Werror",
      "-Wnon-virtual-dtor",
      "-Woverloaded-virtual",
      "-Wold-style-cast",
      "-Wno-overloaded-virtual",
      "-Wvla",
      "-std=c++11",
  ]

def lightstep_linkopts():
  return []

def lightstep_include_prefix(path):
    if path.startswith('src/') or path.startswith('include/'):
        return '/'.join(path.split('/')[1:])
    return None

def lightstep_cc_library(name,
                     srcs = [],
                     hdrs = [],
                     private_hdrs = [],
                     copts = [],
                     linkopts = [],
                     includes = [],
                     visibility = None,
                     external_deps = [],
                     deps = [],
                     data = [],
                     is_3rd_party = False,
                     strip_include_prefix = None):
  native.cc_library(
      name = name,
      srcs = srcs + private_hdrs,
      hdrs = hdrs,
      copts = lightstep_include_copts() + lightstep_copts(is_3rd_party) + copts,
      linkopts = lightstep_linkopts() + linkopts,
      includes = includes,
      deps = external_deps + deps,
      data = data,
      linkstatic = 1,
      include_prefix = lightstep_include_prefix(native.package_name()),
      visibility = visibility,
      strip_include_prefix = strip_include_prefix,
  )

def lightstep_cc_binary(
        name,
        args = [],
        srcs = [],
        data = [],
        copts = [],
        linkopts = [],
        testonly = 0,
        linkshared = 0,
        visibility = None,
        external_deps = [],
        deps = []):
    native.cc_binary(
        name = name,
        args = args,
        srcs = srcs,
        data = data,
        copts = lightstep_include_copts() + lightstep_copts() + copts,
        linkshared = linkshared,
        linkopts = lightstep_linkopts() + linkopts,
        testonly = testonly,
        linkstatic = 1,
        visibility = visibility,
        stamp = 1,
        deps = external_deps + deps,
    )

def lightstep_portable_cc_binary(
        name,
        args = [],
        srcs = [],
        data = [],
        copts = [],
        linkopts = [],
        linkshared = False,
        testonly = 0,
        visibility = None,
        external_deps = [],
        deps = []):
  lightstep_cc_binary(
      name,
      args = args,
      srcs = srcs,
      data = data,
      copts = copts,
      linkopts = linkopts,
      linkshared = linkshared,
      testonly = testonly,
      visibility = visibility,
      external_deps = external_deps,
      deps = deps + select({
          "@com_lightstep_tracer_cpp//bazel:portable_glibc_build": [
              "@com_lightstep_tracer_cpp//bazel:glibc_version_lib",
          ],
          "//conditions:default": []
      }),
  )


def lightstep_cc_test(
        name,
        args = [],
        srcs = [],
        data = [],
        testonly = 0,
        visibility = None,
        external_deps = [],
        deps = [],
        tags = [],
        linkopts = []):
    native.cc_test(
        name = name,
        args = args,
        srcs = srcs,
        data = data,
        copts = lightstep_include_copts() + lightstep_copts(),
        linkopts = lightstep_linkopts() + linkopts,
        testonly = testonly,
        linkstatic = 1,
        tags = tags,
        visibility = visibility,
        stamp = 1,
        deps = external_deps + deps,
    )

def lightstep_catch_test(
        name,
        srcs = [],
        data = [],
        visibility = None,
        external_deps = [],
        deps = [],
        tags = [],
        linkopts = []):
    lightstep_cc_test(
        name = name,
        srcs = srcs,
        data = data,
        tags = tags,
        linkopts = linkopts,
        visibility = visibility,
        deps = deps,
        external_deps = external_deps + ["//3rd_party/catch2:main_lib"],
    )

def lightstep_google_benchmark(
        name,
        srcs = [],
        data = [],
        visibility = None,
        external_deps = [],
        deps = [],
        linkopts = []):
  native.cc_binary(
      name = name,
      srcs = srcs,
      copts = lightstep_include_copts() + lightstep_copts(),
      data = data,
      deps = deps + ["@com_google_benchmark//:benchmark"],
      testonly = 1
  )
  native.cc_test(
      name = name + "_test",
      srcs = srcs,
      args = ["--benchmark_min_time=0"],
      data = data,
      copts = lightstep_include_copts() + lightstep_copts(),
      linkopts = lightstep_linkopts(),
      linkstatic = 1,
      deps = deps + ["@com_google_benchmark//:benchmark"],
  )
  native.genrule(
      name = name + "_result",
      outs = [
          name + "_result.txt",
      ],
      tools = [":" + name] + data,
      testonly = 1,
      cmd = "$(location :%s) --benchmark_color=false --benchmark_min_time=.01 &> $@" % name,
)

def lightstep_go_binary(
    name,
    srcs = [],
    out = None,
    deps = [],
    external_deps = []):
  go_binary(
      name,
      cgo = True,
      # Work around to issues when building go code when sanitizers are enabled.
      # See https://github.com/bazelbuild/rules_go/issues/1306
      clinkopts = select({
          "@com_lightstep_tracer_cpp//bazel:asan_build": [
              "-fsanitize=address",
          ],
          "//conditions:default": []
      }) + select({
          "@com_lightstep_tracer_cpp//bazel:tsan_build": [
              "-fsanitize=thread",
          ],
          "//conditions:default": []
      }),
      srcs = srcs,
      out = out,
      deps = deps + external_deps,
  )
