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
                     alwayslink = True,
                     is_3rd_party = False,
                     strip_include_prefix = None):
  native.cc_library(
      name = name,
      srcs = srcs + private_hdrs,
      hdrs = hdrs,
      copts = lightstep_include_copts() + lightstep_copts(is_3rd_party) + copts,
      linkopts = lightstep_linkopts() + linkopts,
      includes = includes,
      deps = ["//src/common:version_check_lib"] + external_deps + deps,
      data = data,
      linkstatic = 1,
      include_prefix = lightstep_include_prefix(native.package_name()),
      visibility = visibility,
      alwayslink = alwayslink,
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
  native.genrule(
      name = name + "_profile",
      outs = [
          name + "_profile.tgz",
      ],
      tools = [
          ":" + name,
          "//bazel:profile_benchmark",
      ] + data + [
          "@org_valgrind//:valgrind_filegroup",
          "@org_graphviz//:dot_filegroup",
          "//bazel:run_gprof2dot",
      ],
      testonly = 1,
      cmd = "$(location //bazel:profile_benchmark) $$PWD/$(location :%s) $@ " % name +
            "$$PWD/$(location @org_valgrind//:valgrind_filegroup) " +
            "$$PWD/$(location @org_graphviz//:dot_filegroup) " +
            "$$PWD/$(location //bazel:run_gprof2dot)",
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

def lightstep_python_wheel(
    python_tag,
    abi_tag,
    binary):
  outfile = "wheel-%s-%s.tgz" % (python_tag, abi_tag)
  native.genrule(
      name = "gen_wheel-%s-%s" % (python_tag, abi_tag),
      srcs = [
        "//:gen-config/lightstep/version.h",
        "//:LICENSE",
        "//3rd_party/base64:LICENSE",
        "//bridge/python:wheel_files",
        binary,
      ],
      outs = [
          outfile,
      ],
      cmd = """
      PYTHON_TAG=%s
      ABI_TAG=%s
      for file in $(locations //bridge/python:wheel_files); do
        WHEEL_FILES=$${PWD}/`dirname $$file`
      done
      WORK_DIR=`mktemp -d`
      WHEEL_DIR=$$WORK_DIR/wheel
      mkdir $$WHEEL_DIR
      GENERATE_RECORD=$$WHEEL_FILES/generate_record.py
      VERSION_H=$(location //:gen-config/lightstep/version.h)
      VERSION_=`grep LIGHTSTEP_VERSION $$VERSION_H | cut -d ' ' -f3`
      export VERSION=`eval echo $$VERSION_`
      WHEEL_NAME=lightstep_streaming-$$VERSION-$$PYTHON_TAG-$$ABI_TAG-manylinux1_x86_64

      # Set up the dist-info directory
      DISTINFO_NAME=lightstep_streaming-$$VERSION.dist-info
      DISTINFO_DIR=$$WORK_DIR/$$DISTINFO_NAME
      mkdir $$DISTINFO_DIR
      cat $$WHEEL_FILES/WHEEL.in > $$DISTINFO_DIR/WHEEL
      echo "Tag: $$PYTHON_TAG-$$ABI_TAG-manylinux1_x86_64" >> $$DISTINFO_DIR/WHEEL
      cat $$WHEEL_FILES/METADATA.in | envsubst > $$DISTINFO_DIR/METADATA
      echo lightstep_streaming > $$DISTINFO_DIR/top_level.txt

      # Set up the source directory
      SRC_DIR=$$WORK_DIR/lightstep_streaming
      mkdir $$SRC_DIR
      cp $$WHEEL_FILES/__init__.py $$SRC_DIR
      cp $(location %s) $$SRC_DIR
      cp $(location //:LICENSE) $$SRC_DIR
      mkdir -p $$SRC_DIR/3rd_party/base64
      cp $(location //3rd_party/base64:LICENSE) $$SRC_DIR/3rd_party/base64

      # Generate the record file
      python $$GENERATE_RECORD $$WORK_DIR > $$DISTINFO_DIR/RECORD
      echo "$$DISTINFO_NAME/RECORD,," >> $$DISTINFO_DIR/RECORD
      
      # zip up our wheel
      pushd $$WORK_DIR
      zip -r wheel/$$WHEEL_NAME.whl $$DISTINFO_NAME lightstep_streaming
      tar czf wheel.tgz wheel
      popd
      cp $$WORK_DIR/wheel.tgz $${PWD}/$(location :%s)

      rm -rf $$WORK_DIR
      """% (python_tag, abi_tag, binary, outfile),
  )
