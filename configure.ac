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
AM_INIT_AUTOMAKE([1.15 foreign subdir-objects])
AC_CONFIG_SRCDIR([src/c++11])
AC_CONFIG_HEADERS([src/c++11/config.h])
AC_CONFIG_MACRO_DIRS([m4])
AC_ENABLE_STATIC()
AM_DISABLE_SHARED()

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

       dnl cpp-netlib installs itself within the boost directory by default.
       dnl so we presume this is where we'll find it, the AC_CHECK_HEADER below
       dnl does the check. TODO This needs a proper solution.
       AX_BOOST_BASE([1.58], [:], [AC_MSG_ERROR([cpp-netlib requires boost, but not found.])])
       CPP_NETLIB_CPPFLAGS="$BOOST_CPPFLAGS $BOOST_CPPFLAGS/boost"
       CPP_NETLIB_LDFLAGS="$BOOST_LDFLAGS $BOOST_LDFLAGS/boost"
       AC_SUBST(CPP_NETLIB_CPPFLAGS)
       AC_SUBST(CPP_NETLIB_LDFLAGS)
       ])

SAVE_CPPFLAGS="${CPPFLAGS}"
CPPFLAGS="${CPPFLAGS} ${CPP_NETLIB_CPPFLAGS}"

AS_IF([test "$enable_cpp_netlib" != "no"],
      [AC_CHECK_HEADER(network/protocol.hpp,
		[enable_cpp_netlib=yes],
		[AS_IF([test "$enable_cpp_netlib" != "no"],
		       [AC_MSG_ERROR([cpp-netlib required, but not found.])],
  		       [enable_cpp_netlib=no])])]) 

AM_CONDITIONAL([ENABLE_CPP_NETLIB], [test "$enable_cpp_netlib" = "yes"])
AM_CONDITIONAL([ENABLE_OPENSSL], [test "$enable_cpp_netlib" = "yes"])
AC_DEFINE(HAVE_CPP_NETLIB,[$enable_cpp_netlib],
	 [Defined if cpp-netlib is available.])

CPPFLAGS="${SAVE_CPPFLAGS}"

AC_LANG_POP([C++])

AC_CHECK_HEADERS([unistd.h])

AC_CONFIG_FILES([Makefile src/Makefile test/Makefile src/c++11/Makefile test/c++11/Makefile])
AC_OUTPUT
