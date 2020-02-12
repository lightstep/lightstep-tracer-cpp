load("@bazel_tools//tools/cpp:cc_configure.bzl", 
     "cc_autoconf_toolchains",
     _upstream_cc_autoconf_impl = "cc_autoconf_impl")
load("@bazel_tools//tools/cpp:lib_cc_configure.bzl", "get_cpu_value")
load("@bazel_tools//tools/cpp:unix_cc_configure.bzl", "find_cc")

# Stub for `repository_ctx.which()` that always succeeds. See comments in
# `_find_cxx` for details.
def _quiet_fake_which(program):
    return struct(_proj_fake_which = program)

# Stub for `repository_ctx.which()` that always fails. See comments in
# `_find_cxx` for details.
def _noisy_fake_which(program):
    return None

# Find a good path for the C++ compiler, by hooking into Bazel's C compiler
# detection. Uses `$CXX` if found, otherwise defaults to `g++` because Bazel
# defaults to `gcc`.
def _find_cxx(repository_ctx):
    # Bazel's `find_cc` helper uses the repository context to inspect `$CC`.
    # Replace this value with `$CXX` if set.
    environ_cxx = repository_ctx.os.environ.get("CXX", "g++")
    fake_os = struct(
        environ = {"CC": environ_cxx},
    )

    # We can't directly assign `repository_ctx.which` to a struct attribute
    # because Skylark doesn't support bound method references. Instead, stub
    # out `which()` using a two-pass approach:
    #
    # * The first pass uses a stub that always succeeds, passing back a special
    #   value containing the original parameter.
    # * If we detect the special value, we know that `find_cc` found a compiler
    #   name but don't know if that name could be resolved to an executable path.
    #   So do the `which()` call ourselves.
    # * If our `which()` failed, call `find_cc` again with a dummy which that
    #   always fails. The error raised by `find_cc` will be identical to what Bazel
    #   would generate for a missing C compiler.
    #
    # See https://github.com/bazelbuild/bazel/issues/4644 for more context.
    real_cxx = find_cc(struct(
        which = _quiet_fake_which,
        os = fake_os,
    ), {})
    if hasattr(real_cxx, "_proj_fake_which"):
        real_cxx = repository_ctx.which(real_cxx._proj_fake_which)
        if real_cxx == None:
            find_cc(struct(
                which = _noisy_fake_which,
                os = fake_os,
            ), {})
    return real_cxx

def _build_proj_cc_wrapper(repository_ctx):
    real_cc = find_cc(repository_ctx, {})
    real_cxx = _find_cxx(repository_ctx)

    # Copy our CC wrapper script into @local_config_cc, with the true paths
    # to the C and C++ compiler injected in. The wrapper will use these paths
    # to invoke the compiler after deciding which one is correct for the current
    # invocation.
    #
    # Since the script is Python, we can inject values using `repr(str(value))`
    # and escaping will be handled correctly.
    repository_ctx.template("extra_tools/proj_cc_wrapper", repository_ctx.attr._proj_cc_wrapper, {
        "{PROJ_REAL_CC}": repr(str(real_cc)),
        "{PROJ_REAL_CXX}": repr(str(real_cxx)),
    })

    # repository_ctx.template("extra_tools/sanitize-blacklist.txt", repository_ctx.attr._proj_sanitize_blacklist, {
    # })

    return repository_ctx.path("extra_tools/proj_cc_wrapper")

def _needs_proj_cc_wrapper(repository_ctx):
    # When building for Linux we set additional C++ compiler options that aren't
    # handled well by Bazel, so we need a wrapper around $CC to fix its
    # compiler invocations.
    cpu_value = get_cpu_value(repository_ctx)
    return cpu_value not in ["freebsd", "x64_windows", "darwin"]

def cc_autoconf_impl(repository_ctx):
    overriden_tools = {}
    if _needs_proj_cc_wrapper(repository_ctx):
        # Bazel uses "gcc" as a generic name for all C and C++ compilers.
        overriden_tools["gcc"] = _build_proj_cc_wrapper(repository_ctx)
    return _upstream_cc_autoconf_impl(repository_ctx, overriden_tools = overriden_tools)

MSVC_ENVVARS = [
    "BAZEL_VC",
    "BAZEL_VC_FULL_VERSION",
    "BAZEL_VS",
    "BAZEL_WINSDK_FULL_VERSION",
    "VS90COMNTOOLS",
    "VS100COMNTOOLS",
    "VS110COMNTOOLS",
    "VS120COMNTOOLS",
    "VS140COMNTOOLS",
    "VS150COMNTOOLS",
    "VS160COMNTOOLS",
    "TMP",
    "TEMP",
]

cc_autoconf = repository_rule(
    attrs = {
        "_proj_cc_wrapper": attr.label(default = "@com_lightstep_tracer_cpp//bazel:cc_wrapper.py"),
    },
    environ = [
        "ABI_LIBC_VERSION",
        "ABI_VERSION",
        "BAZEL_COMPILER",
        "BAZEL_HOST_SYSTEM",
        "BAZEL_CXXOPTS",
        "BAZEL_LINKOPTS",
        "BAZEL_LINKLIBS",
        "BAZEL_PYTHON",
        "BAZEL_SH",
        "BAZEL_TARGET_CPU",
        "BAZEL_TARGET_LIBC",
        "BAZEL_TARGET_SYSTEM",
        "BAZEL_USE_CPP_ONLY_TOOLCHAIN",
        "BAZEL_USE_XCODE_TOOLCHAIN",
        "BAZEL_DO_NOT_DETECT_CPP_TOOLCHAIN",
        "BAZEL_USE_LLVM_NATIVE_COVERAGE",
        "BAZEL_LLVM",
        "BAZEL_IGNORE_SYSTEM_HEADERS_VERSIONS",
        "USE_CLANG_CL",
        "CC",
        "CC_CONFIGURE_DEBUG",
        "CC_TOOLCHAIN_NAME",
        "CPLUS_INCLUDE_PATH",
        "GCOV",
        "HOMEBREW_RUBY_PATH",
        "SYSTEMROOT",
    ] + MSVC_ENVVARS,
    implementation = cc_autoconf_impl,
    configure = True,
)

def cc_configure():
    cc_autoconf_toolchains(name = "local_config_cc_toolchains")
    cc_autoconf(name = "local_config_cc")
    native.bind(name = "cc_toolchain", actual = "@local_config_cc//:toolchain")
    native.register_toolchains(
        # Use register_toolchain's target pattern expansion to register all toolchains in the package.
        "@local_config_cc_toolchains//:all",
    )
