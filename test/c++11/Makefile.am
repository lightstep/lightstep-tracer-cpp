noinst_PROGRAMS = tracer_test

tracer_test_SOURCES = tracer_test.cc
tracer_test_CXXFLAGS = -I ../../src/c++11
tracer_test_LDADD = ../../src/c++11/liblightstep_cxx11.la

if ENABLE_CPP_NETLIB
noinst_PROGRAMS += cppclient

cppclient_SOURCES = cppclient.cc zintinio_happyhttp/happyhttp.cpp
cppclient_CXXFLAGS = -I ../../src/c++11 -I ../../src/c++11/dropbox_json -I zintinio_happyhttp
cppclient_LDADD = ../../src/c++11/liblightstep_cxx11.la
endif
