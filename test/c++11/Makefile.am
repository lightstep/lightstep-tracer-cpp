# TODO This should be defined and AC_SUBST in configure.ac but having trouble
# getting the value of $(top_srcdir)??
TEST_CXXFLAGS = $(AM_CXXFLAGS) \
		-I$(top_srcdir)/vendor/cpp-netlib \
		$(BOOST_CPPFLAGS)
TEST_LDFLAGS  = -L$(top_srcdir)/vendor/cpp-netlib/build/libs/network/src \
		$(BOOST_LDFLAGS) \
		-lboost_system -lcppnetlib-uri -lcppnetlib-client-connections
#  -I ../../src/c++11/dropbox_json -I zintinio_happyhttp

noinst_PROGRAMS = tracer_test

tracer_test_SOURCES = tracer_test.cc
tracer_test_CXXFLAGS = -I ../../src/c++11  $(TEST_CPPFLAGS)
tracer_test_LDADD = ../../src/c++11/liblightstep_cxx11.la
tracer_test_LDFLAGS = -static $(TEST_LDFLAGS)

if ENABLE_CPP_NETLIB
noinst_PROGRAMS += cppclient

cppclient_SOURCES = cppclient.cc zintinio_happyhttp/happyhttp.cpp
cppclient_CXXFLAGS = -I ../../src/c++11  $(TEST_CPPFLAGS)
cppclient_LDADD = ../../src/c++11/liblightstep_cxx11.la
cppclient_LDFLAGS = -static $(TEST_LDFLAGS)
endif
