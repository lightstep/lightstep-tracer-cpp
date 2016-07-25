# -*- Autoconf -*-
#
# TODO On Mac OS X building against "keg-only" libraries (e.g., the
# Thrift you built depends on a keg-only openssl) you'll have to
# ammend your pkg-config path:
#
#   export PKG_CONFIG_PATH=$(brew --prefix)/opt/openssl/lib/pkgconfig
#
AC_PREREQ([2.69])
AC_INIT([LightStep C++ Tracer], [0.9],
        [https://github.com/lightstep/lightstep-tracer-cpp/issues])
LT_INIT
AM_INIT_AUTOMAKE([1.14 foreign subdir-objects])
AC_CONFIG_SRCDIR([src/c++11])
AC_CONFIG_HEADERS([src/c++11/config.h])
AC_CONFIG_MACRO_DIRS([m4])

AC_LANG_PUSH([C++])
AC_PROG_CXX
AX_CXX_COMPILE_STDCXX([11])

AC_ARG_ENABLE([cpp-netlib],
  [AS_HELP_STRING([--disable-cpp-netlib],
  [Enable LightStep default networking @<:@yes@:>@])],
  [:],
  [enable_cpp_netlib=check])

AS_IF([test "$enable_cpp_netlib" != "no"],
      [dnl OpenSSL is needed only when networking is used, but it's possible
       dnl a user would like networking without SSL support. Optional?
       PKG_CHECK_MODULES(openssl, openssl >= 0.9.8zg)
       AC_SUBST(openssl_LIBS)
       AC_SUBST(openssl_CFLAGS)

       AC_DEFINE(BOOST_NETWORK_ENABLE_HTTPS, [], [With HTTPS support])
       AX_BOOST_BASE([1.55], [:], [AC_MSG_ERROR([cpp-netlib requires boost, but not found.])])
       enable_cpp_netlib=yes])

AM_CONDITIONAL([ENABLE_CPP_NETLIB], [test "$enable_cpp_netlib" = "yes"])
AM_CONDITIONAL([ENABLE_OPENSSL], [test "$enable_cpp_netlib" = "yes"])

AS_IF([test "$enable_cpp_netlib" == "yes"],
      [AC_DEFINE(HAVE_CPP_NETLIB,[],[Defined if cpp-netlib is available.])])

AC_LANG_POP([C++])

AC_CHECK_HEADERS([unistd.h])

AC_PATH_PROG([CMAKE], [cmake], [/usr/bin/cmake])

AC_CONFIG_FILES([Makefile src/Makefile test/Makefile src/c++11/Makefile test/c++11/Makefile vendor/Makefile])
AC_OUTPUT
