# -*- Autoconf -*-
#
# TODO On Mac OS X building against "keg-only" libraries (e.g., the
# Thrift you built depends on a keg-only openssl) you'll have to
# ammend your pkg-config path:
#
#   export PKG_CONFIG_PATH=$(brew --prefix)/opt/openssl/lib/pkgconfig
#
AC_PREREQ([2.69])
AC_INIT([LightStep C++ Tracer], [0.8],
        [https://github.com/lightstep/lightstep-tracer-cpp/issues])
LT_INIT
AM_INIT_AUTOMAKE([1.15 foreign subdir-objects])
AC_CONFIG_SRCDIR([src])
AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_MACRO_DIRS([m4])

AC_PROG_CXX
AX_CXX_COMPILE_STDCXX([11])

PKG_CHECK_MODULES(thrift, thrift >= 0.9.2)
AC_SUBST(thrift_LIBS)
AC_SUBST(thrift_CFLAGS)

PKG_CHECK_MODULES(openssl, openssl >= 0.9.8zg)
AC_SUBST(openssl_LIBS)
AC_SUBST(openssl_CFLAGS)

AC_CHECK_HEADERS([unistd.h])

AC_CONFIG_FILES([Makefile src/Makefile test/Makefile])
AC_OUTPUT
