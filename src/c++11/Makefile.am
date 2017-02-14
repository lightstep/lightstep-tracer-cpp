# Lightstep C++ Tracer core and gRPC recorder libraries
#
# To set the protocol compiler binary, use e.g., PROTOC
#
#   ./configure PROTOC=/usr/local/bin/protoc-3.0.1
PROTOC = @PROTOC@
.PHONY: $(PROTOC)

# Lightstep Collector reporting:
COLLECTOR_SRC = $(top_srcdir)/lightstep-tracer-common
ENVOY_SRC = ./envoy
PROTO_DIR = .

PROTO_SRC = \
	$(COLLECTOR_SRC)/collector.proto \
	$(ENVOY_SRC)/envoy_carrier.proto

PROTO_GEN = \
	lightstep/collector.pb.h \
	lightstep/envoy_carrier.pb.h \
	collector.pb.cc \
	envoy_carrier.pb.cc

proto.stamp: $(PROTO_SRC)
	$(PROTOC) --proto_path=$(COLLECTOR_SRC) --cpp_out=$(PROTO_DIR) $(COLLECTOR_SRC)/collector.proto
	$(PROTOC) --proto_path=$(ENVOY_SRC) --cpp_out=$(PROTO_DIR) $(ENVOY_SRC)/envoy_carrier.proto
	mv collector.pb.h envoy_carrier.pb.h lightstep
	touch proto.stamp

$(PROTO_GEN): proto.stamp

# Tell Automake that these files have to be generated first (also cleaned)
BUILT_SOURCES = $(PROTO_GEN)
CLEANFILES    = $(PROTO_GEN) proto.stamp

# Lightstep Tracer core library
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

liblightstep_core_cxx11_la_SOURCES = \
	impl.cc \
	span.cc \
	tracer.cc \
	util.cc

nobase_nodist_include_HEADERS = \
	lightstep/collector.pb.h \
	lightstep/envoy_carrier.pb.h

nodist_liblightstep_core_cxx11_la_SOURCES = \
	collector.pb.cc \
	envoy_carrier.pb.cc

# Note: To set the protobuf library include and library flags, e.g.,
#
#   ./configure protobuf_CFLAGS="..." protobuf_LIBS="..."
liblightstep_core_cxx11_la_LIBADD   =
liblightstep_core_cxx11_la_CXXFLAGS = $(AM_CXXFLAGS) $(protobuf_CFLAGS) -I$(srcdir)/lightstep
liblightstep_core_cxx11_la_LDFLAGS  = $(protobuf_LIBS)

# Options
AM_CXXFLAGS = -Wno-deprecated-declarations

EXTRA_DIST = \
	mapbox_variant/LICENSE \
	envoy/envoy_carrier.proto

# The builtin gRPC transport is built by default.  When supplying your
# own gRPC transport, you can disable the built-in recorder:
#
#   ./configure --disable-grpc
if ENABLE_GRPC

GRPC_PROTO_SRC = \
	$(COLLECTOR_SRC)/collector.proto

GRPC_PROTO_GEN = \
	lightstep/collector.grpc.pb.h \
	collector.grpc.pb.cc

BUILT_SOURCES += $(GRPC_PROTO_GEN)
CLEANFILES    += $(GRPC_PROTO_GEN) proto.grpc.stamp

# TODO make grpc_cpp_plugin a configure option
#
proto.grpc.stamp: $(GRPC_PROTO_SRC)
	$(PROTOC) --proto_path=$(COLLECTOR_SRC) --grpc_out=$(PROTO_DIR) \
		--plugin=protoc-gen-grpc=`which grpc_cpp_plugin` \
		$(COLLECTOR_SRC)/collector.proto
	mv collector.grpc.pb.h lightstep
	touch proto.grpc.stamp

$(GRPC_PROTO_GEN): proto.grpc.stamp

# LightStep gRPC recorder library
lib_LTLIBRARIES += liblightstep_grpc_cxx11.la

liblightstep_grpc_cxx11_la_SOURCES = \
	recorder.cc

nobase_nodist_include_HEADERS += \
	lightstep/collector.grpc.pb.h

nodist_liblightstep_grpc_cxx11_la_SOURCES = \
	collector.grpc.pb.cc

# Note $(grpc_LIBS)++ shenanigans. The grpc C++ pkg-config input (grpc++.pc) uses
# characters that are invalid to autoconf, so we can't find it.  Instead, we locate
# the C library and tack on a '++'.
liblightstep_grpc_cxx11_la_LIBADD   = liblightstep_core_cxx11.la
liblightstep_grpc_cxx11_la_CXXFLAGS = $(AM_CXXFLAGS) $(grpc_CFLAGS) $(protobuf_CFLAGS) -I$(srcdir)/lightstep
liblightstep_grpc_cxx11_la_LDFLAGS  = $(protobuf_LIBS) $(grpc_LIBS)++
endif
