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
AC_CONFIG_SRCDIR([src/c++11])
AC_CONFIG_HEADERS([src/c++11/config.h])
AC_CONFIG_MACRO_DIRS([m4])
AC_ENABLE_STATIC()
AM_DISABLE_SHARED()

AC_PROG_CXX
AX_CXX_COMPILE_STDCXX([11])

# This will return:
# PKG_CHECK_MODULES(openssl, openssl >= 0.9.8zg)
# AC_SUBST(openssl_LIBS)
# AC_SUBST(openssl_CFLAGS)

AC_ARG_ENABLE([cpp-network],
  [AS_HELP_STRING([--disable-cpp-network],
  [Disable LightStep default networking @<:@check@:>@])],
  [:],
  [enable_cpp_network=check])

AS_IF([test "$enable_cpp_network" != "no"],
      [AC_CHECK_HEADER(boost/network/protocol.hpp,
		[enable_cpp_network=yes],
		[AS_IF([test "$enable_cpp_network" != "no"],
	        [AC_MSG_ERROR([cpp-network required, but not found.])],
		      [enable_cpp_network=no])])])

AC_CHECK_HEADERS([unistd.h])

AC_CONFIG_FILES([Makefile src/Makefile test/Makefile src/c++11/Makefile test/c++11/Makefile])
AC_OUTPUT
