noinst_PROGRAMS = tracer_test cppclient

tracer_test_SOURCES = tracer_test.cc
tracer_test_CXXFLAGS = -I ../../src/c++11
tracer_test_LDADD = ../../src/c++11/liblightstep_cpp.la

cppclient_SOURCES = cppclient.cc dropbox_json/json11.cpp zintinio_happyhttp/happyhttp.cpp
cppclient_CXXFLAGS = -I ../../src/c++11 -I dropbox_json -I zintinio_happyhttp
cppclient_LDADD = ../../src/c++11/liblightstep_cpp.la
