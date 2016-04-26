.PHONY: clean thrift

HEADERS=lightstep_thrift/lightstep_constants.h \
	lightstep_thrift/lightstep_types.h

INCLUDE=/usr/local/include

CFLAGS=-g -I$(INCLUDE)

%.o: %.cpp $(HEADERS)
	echo $(HEADERS)
	$(CXX) -std=c++11 -c -o $@ $< $(CFLAGS)

OBJECTS=lightstep_thrift/lightstep_constants.o \
	lightstep_thrift/lightstep_types.o

all: $(OBJECTS)

clean:
	rm $(OBJECTS)

# LightStep-specific: rebuilds the LightStep thrift protocol files.  Assumes
# the command is run within the LightStep development environment (i.e. the
# LIGHTSTEP_HOME environment variable is set).
thrift:
	mkdir -p lightstep_thrift
	rm -f lightstep_thrift/*
	cp $(LIGHTSTEP_HOME)/go/src/crouton/crouton.thrift lightstep_thrift/lightstep.thrift
	thrift -r -gen cpp -out lightstep_thrift/ lightstep_thrift/lightstep.thrift
	rm lightstep_thrift/ReportingService.cpp \
		lightstep_thrift/ReportingService.h \
		lightstep_thrift/ReportingService_server.skeleton.cpp \
		lightstep_thrift/lightstep.thrift
