# -*- Autoconf -*-
#
# TODO On Mac OS X building against "keg-only" libraries installed
# by homebrew, see the commands to set PKG_CONFIG_PATH in build.sh.

AC_PREREQ([2.69])
AC_INIT([lightstep-tracer-cpp], [0.34],
        [https://github.com/lightstep/lightstep-tracer-cpp/issues])
LT_INIT([disable-shared])
AM_INIT_AUTOMAKE([1.14 foreign subdir-objects])
AC_CONFIG_SRCDIR([src/c++11])
AC_CONFIG_HEADERS([src/c++11/config.h])
AC_CONFIG_MACRO_DIRS([m4])

AC_LANG_PUSH([C++])
AC_PROG_CXX
AX_CXX_COMPILE_STDCXX([11])

AS_IF([test -z "${protobuf_LIBS}"],
      [PKG_CHECK_MODULES(protobuf, protobuf >= 3.0.0)])

SAVE_CPPFLAGS="${CPPFLAGS}"
CPPFLAGS="${CPPFLAGS} ${protobuf_CFLAGS}"

AC_CHECK_HEADER(google/protobuf/message.h)

CPPFLAGS="${SAVE_CPPFLAGS}"

AC_SUBST(protobuf_LIBS)
AC_SUBST(protobuf_CFLAGS)

AC_CACHE_VAL(protobuf_cv_PREFIX, [protobuf_cv_PREFIX=`$PKG_CONFIG --variable=prefix protobuf`])

AS_IF([test -z "${PROTOC}"],
      [PROTOC="$protobuf_cv_PREFIX/bin/protoc"])

AC_ARG_VAR(PROTOC, "Location of the protocol compiler")

AC_ARG_ENABLE([grpc],
  [AS_HELP_STRING([--disable-grpc],
  [Enable gRPC networking @<:@yes@:>@])],
  [:],
  [enable_grpc=check])

AS_IF([test "${enable_grpc}" != "no"],
      [AS_IF([test -z "${grpc_LIBS}"],
	  [PKG_CHECK_MODULES(grpc, grpc >= 1.0.0)])
       AC_SUBST(grpc_LIBS)
       AC_SUBST(grpc_CFLAGS)
       ])

SAVE_CPPFLAGS="${CPPFLAGS}"
CPPFLAGS="${CPPFLAGS} ${grpc_CFLAGS}"

AS_IF([test "$enable_grpc" != "no"],
      [AC_CHECK_HEADER(grpc++/grpc++.h,
		[AC_DEFINE(HAVE_GRPC,[],[Defined if grpc is available])
       		 enable_grpc=yes],
		[AS_IF([test "$enable_grpc" != "no"],
	        [AC_MSG_ERROR([gRPC support requested, but not found.])],
		      [enable_grpc=no])])])

CPPFLAGS="${SAVE_CPPFLAGS}"

AM_CONDITIONAL([ENABLE_GRPC], [test "$enable_grpc" = "yes"])

AC_LANG_POP([C++])

AC_CHECK_HEADERS([unistd.h])

AC_CONFIG_FILES([Makefile src/Makefile test/Makefile src/c++11/Makefile test/c++11/Makefile])
AC_OUTPUT
