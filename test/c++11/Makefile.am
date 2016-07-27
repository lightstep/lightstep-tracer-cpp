
noinst_PROGRAMS = nonetwork_test

nonetwork_test_SOURCES = nonetwork_test.cc
nonetwork_test_CXXFLAGS = -I ../../src/c++11
nonetwork_test_LDADD = ../../src/c++11/liblightstep_nonetwork_cxx11.la
nonetwork_test_LDFLAGS = -static

if ENABLE_CPP_NETLIB

TESTCXXFLAGS = $(AM_CXXFLAGS) \
		-I$(top_srcdir)/vendor/cpp-netlib \
		-I$(top_srcdir)/vendor/boost_1_60_0/build/include
TESTLDFLAGS  = -L$(top_srcdir)/vendor/cpp-netlib/build/libs/network/src \
		-L$(top_srcdir)/vendor/boost_1_60_0/build/lib \
		-lboost_system -lcppnetlib-uri -lcppnetlib-client-connections

noinst_PROGRAMS += tracer_test

tracer_test_SOURCES = tracer_test.cc
tracer_test_CXXFLAGS = -I ../../src/c++11  $(TESTCPPFLAGS)
tracer_test_LDADD = ../../src/c++11/liblightstep_cxx11.la
tracer_test_LDFLAGS = -static $(TESTLDFLAGS)

noinst_PROGRAMS += cppclient

cppclient_SOURCES = cppclient.cc zintinio_happyhttp/happyhttp.cpp
cppclient_CXXFLAGS = -I ../../src/c++11  $(TESTCPPFLAGS)
cppclient_LDADD = ../../src/c++11/liblightstep_cxx11.la
cppclient_LDFLAGS = -static $(TESTLDFLAGS)

endif
