
noinst_PROGRAMS =

noinst_PROGRAMS += flushproto_test

flushproto_test_SOURCES = flushproto_test.cc
flushproto_test_CXXFLAGS = -I ../../src/c++11 $(protobuf_CFLAGS)
flushproto_test_LDADD = ../../src/c++11/liblightstep_core_cxx11.la $(protobuf_LIBS)
flushproto_test_LDFLAGS =

if ENABLE_GRPC

noinst_PROGRAMS += tracer_test

tracer_test_SOURCES = tracer_test.cc
tracer_test_CXXFLAGS = -I ../../src/c++11
tracer_test_LDADD = ../../src/c++11/liblightstep_grpc_cxx11.la
tracer_test_LDFLAGS = -static

noinst_PROGRAMS += cppclient

cppclient_SOURCES = \
	cppclient.cc \
	dropbox_json/json11.cpp \
	zintinio_happyhttp/happyhttp.cpp

cppclient_CXXFLAGS = -I ../../src/c++11
cppclient_LDADD = ../../src/c++11/liblightstep_grpc_cxx11.la
cppclient_LDFLAGS = -static

endif
