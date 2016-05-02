# TODO will need to autoconf this to find Thrift and OpenSSL

.PHONY: clean thrift

HEADERS=\
	impl.h \
	options.h \
	span.h \
	tracer.h \
	value.h \
	lightstep_thrift/lightstep_constants.h \
	lightstep_thrift/lightstep_types.h \
	mapbox_variant/variant.hpp

# Where to find the Thrift, OpenSSL libraries.
INCLUDE=-I/Users/jmacd/src/thrift/lib/cpp/src -I/usr/local/Cellar/openssl/1.0.2g/include -I/usr/local/include
LDFLAGS=-L/Users/jmacd/src/thrift/lib/cpp/.libs -L/usr/local/Cellar/openssl/1.0.2g/lib -L/usr/local/lib

CXXFLAGS=-g $(INCLUDE) -Wall
LIBS=-lthrift -lc++

%.o: %.cpp $(HEADERS)
	$(CXX) -c -std=c++11 $(CXXFLAGS) -o $@ $<

%.o: %.cc $(HEADERS)
	$(CXX) -c -std=c++11 $(CXXFLAGS) -o $@ $<

OBJECTS=\
	impl.o \
	recorder.o \
	span.o \
	tracer.o \
	util.o \
	lightstep_thrift/lightstep_constants.o \
	lightstep_thrift/lightstep_service.o \
	lightstep_thrift/lightstep_types.o

all: $(OBJECTS)

clean:
	@rm -f $(OBJECTS)

test: test.cc $(OBJECTS)
	$(CC) -std=c++11 -o test test.cc $(CXXFLAGS) $(LDFLAGS) $(OBJECTS) $(LIBS)

# LightStep-specific: rebuilds the LightStep thrift protocol files.  Assumes
# the command is run within the LightStep development environment (i.e. the
# LIGHTSTEP_HOME environment variable is set).
#
# TODO Note that the "moveable_types" option is not functioning and
# needs to be fixed.
thrift:
	mkdir -p lightstep_thrift
	rm -f lightstep_thrift/*
	cp $(LIGHTSTEP_HOME)/go/src/crouton/crouton.thrift lightstep_thrift/lightstep.thrift
	thrift -r -gen cpp:moveable_types -out lightstep_thrift/ lightstep_thrift/lightstep.thrift
	sed s/ReportingService.h/lightstep_service.h/ lightstep_thrift/ReportingService.cpp > lightstep_thrift/lightstep_service.cc
	mv lightstep_thrift/ReportingService.h lightstep_thrift/lightstep_service.h
	rm lightstep_thrift/ReportingService.cpp
	rm lightstep_thrift/ReportingService_server.skeleton.cpp
	rm lightstep_thrift/lightstep.thrift
