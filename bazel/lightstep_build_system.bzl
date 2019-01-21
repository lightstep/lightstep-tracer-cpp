def lightstep_package():
    native.package(default_visibility = ["//visibility:public"])

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


def lightstep_include_prefix(path):
    if path.startswith('src/') or path.startswith('include/'):
        return '/'.join(path.split('/')[1:])
    return None

def lightstep_cc_library(name,
                     srcs = [],
                     hdrs = [],
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
      srcs = srcs,
      hdrs = hdrs,
      copts = lightstep_copts(is_3rd_party) + copts,
      linkopts = linkopts,
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
        srcs = [],
        data = [],
        testonly = 0,
        visibility = None,
        external_deps = [],
        deps = [],
        linkopts = []):
    native.cc_binary(
        name = name,
        srcs = srcs,
        data = data,
        copts = lightstep_copts(),
        linkopts = linkopts,
        testonly = testonly,
        linkstatic = 1,
        visibility = visibility,
        stamp = 1,
        deps = external_deps + deps,
    )

def lightstep_cc_test(
        name,
        srcs = [],
        data = [],
        testonly = 0,
        visibility = None,
        external_deps = [],
        deps = [],
        linkopts = []):
    native.cc_test(
        name = name,
        srcs = srcs,
        data = data,
        copts = lightstep_copts(),
        linkopts = linkopts,
        testonly = testonly,
        linkstatic = 1,
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
        linkopts = []):
    lightstep_cc_test(
        name = name,
        srcs = srcs,
        data = data,
        linkopts = linkopts,
        visibility = visibility,
        deps = deps,
        external_deps = external_deps + ["//3rd_party/catch2:main_lib"],
    )
