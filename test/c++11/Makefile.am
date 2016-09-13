
noinst_PROGRAMS = flushproto_test

flushproto_test_SOURCES = flushproto_test.cc
flushproto_test_CXXFLAGS = -I $(top_srcdir)/src/c++11 -I $(top_srcdir)/src/c++11/lightstep $(protobuf_CFLAGS)
flushproto_test_LDADD = ../../src/c++11/liblightstep_core_cxx11.la
flushproto_test_LDFLAGS = $(protobuf_LIBS)

EXTRA_DIST = \
	dropbox_json/LICENSE.txt

noinst_HEADERS = \
	dropbox_json/json11.hpp \
	zintinio_happyhttp/happyhttp.h

if ENABLE_GRPC

noinst_PROGRAMS += tracer_test

tracer_test_SOURCES = tracer_test.cc
tracer_test_CXXFLAGS = -I $(top_srcdir)/src/c++11 -I $(top_srcdir)/src/c++11/lightstep $(grpc_CFLAGS) $(protobuf_CFLAGS)
tracer_test_LDADD = ../../src/c++11/liblightstep_grpc_cxx11.la 
tracer_test_LDFLAGS = $(protobuf_LIBS) $(grpc_LIBS)++

noinst_PROGRAMS += cppclient

cppclient_SOURCES = \
	cppclient.cc \
	dropbox_json/json11.cpp \
	zintinio_happyhttp/happyhttp.cpp

cppclient_CXXFLAGS = -I $(top_srcdir)/src/c++11 -I $(top_srcdir)/src/c++11/lightstep $(grpc_CFLAGS) $(protobuf_CFLAGS)
cppclient_LDADD = ../../src/c++11/liblightstep_grpc_cxx11.la
cppclient_LDFLAGS = $(protobuf_LIBS) $(grpc_LIBS)++

endif
