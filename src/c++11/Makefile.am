# TODO(jmacd): Replace the proto-generation rule with one using the protoc docker image.
PROTOC = @PROTOC@
COLLECTOR_SRC = $(top_srcdir)/lightstep-tracer-common
ENVOY_SRC = ./envoy
PROTO_DIR = proto

PROTO_SRC = \
	$(COLLECTOR_SRC)/collector.proto \
	$(ENVOY_SRC)/envoy_carrier.proto

GRPC_PROTO_SRC = \
	$(COLLECTOR_SRC)/collector.proto

PROTO_GEN = \
	lightstep/collector.pb.h \
	lightstep/envoy_carrier.pb.h \
	proto/collector.pb.cc \
	proto/envoy_carrier.pb.cc

GRPC_PROTO_GEN = \
	lightstep/collector.grpc.pb.h \
	proto/collector.grpc.pb.cc

CLEANFILES = $(PROTO_GEN) $(GRPC_PROTO_GEN)

.PHONY: proto

AM_CXXFLAGS = -Wno-deprecated-declarations

$(PROTO_GEN): $(PROTO_SRC) $(PROTOC)
	mkdir -p $(PROTO_DIR)
	$(PROTOC) --proto_path=$(COLLECTOR_SRC) --cpp_out=$(PROTO_DIR) $(COLLECTOR_SRC)/collector.proto
	$(PROTOC) --proto_path=$(ENVOY_SRC) --cpp_out=$(PROTO_DIR) $(ENVOY_SRC)/envoy_carrier.proto
	mv proto/*.h lightstep

$(GRPC_PROTO_GEN): $(GRPC_PROTO_SRC) $(PROTOC)
	mkdir -p $(PROTO_DIR)
	$(PROTOC) --proto_path=$(COLLECTOR_SRC) --grpc_out=$(PROTO_DIR) --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` $(COLLECTOR_SRC)/collector.proto
	mv proto/*.h lightstep

BUILT_SOURCES = $(PROTO_GEN) $(GRPC_PROTO_GEN)

lib_LTLIBRARIES = liblightstep_core_cxx11.la

nobase_include_HEADERS = \
	lightstep/envoy.h \
	lightstep/impl.h \
	lightstep/options.h \
	lightstep/propagation.h \
	lightstep/recorder.h \
	lightstep/span.h \
	lightstep/tracer.h \
	lightstep/util.h \
	lightstep/value.h \
	mapbox_variant/recursive_wrapper.hpp \
	mapbox_variant/variant.hpp

nobase_nodist_include_HEADERS = \
	lightstep/collector.grpc.pb.h \
	lightstep/collector.pb.h \
	lightstep/envoy_carrier.pb.h

EXTRA_DIST = \
	mapbox_variant/LICENSE \
	envoy/envoy_carrier.proto

liblightstep_core_cxx11_la_SOURCES = \
	impl.cc \
	span.cc \
	tracer.cc \
	util.cc

nodist_liblightstep_core_cxx11_la_SOURCES = \
	proto/collector.pb.cc \
	proto/envoy_carrier.pb.cc

liblightstep_core_cxx11_la_LIBADD   =
liblightstep_core_cxx11_la_CXXFLAGS = $(AM_CXXFLAGS) $(protobuf_CFLAGS) -I$(srcdir)/lightstep
liblightstep_core_cxx11_la_LDFLAGS  = $(protobuf_LIBS)

if ENABLE_GRPC
lib_LTLIBRARIES += liblightstep_grpc_cxx11.la

liblightstep_grpc_cxx11_la_SOURCES = \
	recorder.cc

nodist_liblightstep_grpc_cxx11_la_SOURCES = \
	proto/collector.grpc.pb.cc

# Note $(grpc_LIBS)++ shenanigans. The grpc C++ pkg-config input (grpc++.pc) uses
# characters that are invalid to autoconf, so we can't find it.  Instead, we locate
# the C library and tack on a '++'.
liblightstep_grpc_cxx11_la_LIBADD   = liblightstep_core_cxx11.la
liblightstep_grpc_cxx11_la_CXXFLAGS = $(AM_CXXFLAGS) $(grpc_CFLAGS) $(protobuf_CFLAGS) -I$(srcdir)/lightstep
liblightstep_grpc_cxx11_la_LDFLAGS  = $(protobuf_LIBS) $(grpc_LIBS)++
endif
