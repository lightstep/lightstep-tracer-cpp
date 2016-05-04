noinst_PROGRAMS = tracer_test cppclient

tracer_test_SOURCES = tracer_test.cc
tracer_test_CXXFLAGS = -I ../src
tracer_test_LDADD = ../src/liblightstep.la

cppclient_SOURCES = cppclient.cc dropbox_json/json11.cpp zintinio_happyhttp/happyhttp.cpp
cppclient_CXXFLAGS = -I ../src -I dropbox_json -I zintinio_happyhttp
cppclient_LDADD = ../src/liblightstep.la
