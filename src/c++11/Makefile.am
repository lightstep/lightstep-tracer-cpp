
lib_LTLIBRARIES = liblightstep_cpp.la
liblightstep_cpp_la_SOURCES = \
	impl.cc \
	recorder.cc \
	span.cc \
	tracer.cc \
	util.cc \
	lightstep_thrift/lightstep_constants.cpp \
	lightstep_thrift/lightstep_service.cpp \
	lightstep_thrift/lightstep_types.cpp

liblightstep_cpp_la_CXXFLAGS = $(AM_CXXFLAGS)
liblightstep_cpp_la_LIBADD   =

liblightstep_cpp_la_CXXFLAGS += $(thrift_CFLAGS)
liblightstep_cpp_la_CXXFLAGS += $(openssl_CFLAGS)

liblightstep_cpp_la_LIBADD   += $(thrift_LIBS)
liblightstep_cpp_la_LIBADD   += $(openssl_LIBS)

# LightStep-specific: rebuilds the LightStep thrift protocol files for
# C++11.  Assumes the command is run within the LightStep development
# environment (i.e. the LIGHTSTEP_HOME environment variable is set).
thrift:
	mkdir -p lightstep_thrift
	rm -f lightstep_thrift/*
	cp $(LIGHTSTEP_HOME)/go/src/crouton/crouton.thrift lightstep_thrift/lightstep.thrift
	thrift -r -gen cpp:moveable_types -out lightstep_thrift/ lightstep_thrift/lightstep.thrift
	sed s/ReportingService.h/lightstep_service.h/ lightstep_thrift/ReportingService.cpp > lightstep_thrift/lightstep_service.cpp
	mv lightstep_thrift/ReportingService.h lightstep_thrift/lightstep_service.h
	rm lightstep_thrift/ReportingService.cpp
	rm lightstep_thrift/ReportingService_server.skeleton.cpp
	rm lightstep_thrift/lightstep.thrift
