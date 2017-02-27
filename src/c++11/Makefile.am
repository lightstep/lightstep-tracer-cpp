# Lightstep C++ Tracer core and gRPC recorder libraries
#
# To set the protocol compiler binary, use e.g., PROTOC
#
#   ./configure PROTOC=/usr/local/bin/protoc-3.0.1
PROTOC = @PROTOC@
.PHONY: $(PROTOC)

# Lightstep Collector reporting:
proto_srcdir = $(top_srcdir)/lightstep-tracer-common

PROTO_SRC = \
	$(proto_srcdir)/collector.proto \
	$(proto_srcdir)/lightstep_carrier.proto

PROTO_GEN = \
	collector.pb.h \
	collector.pb.cc \
	lightstep_carrier.pb.h \
	lightstep_carrier.pb.cc

proto.stamp: $(PROTO_SRC)
	$(PROTOC) --proto_path=$(proto_srcdir) --cpp_out=$(builddir) $(proto_srcdir)/collector.proto
	$(PROTOC) --proto_path=$(proto_srcdir) --cpp_out=$(builddir) $(proto_srcdir)/lightstep_carrier.proto
	touch proto.stamp

$(PROTO_GEN): proto.stamp

# Tell Automake that these files have to be generated first (also cleaned)
BUILT_SOURCES = $(PROTO_GEN)
CLEANFILES    = $(PROTO_GEN) proto.stamp

# Lightstep Tracer core library
lib_LTLIBRARIES = liblightstep_core_cxx11.la

nobase_include_HEADERS = \
	lightstep/carrier.h \
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
	collector.pb.h \
	lightstep_carrier.pb.h

nodist_liblightstep_core_cxx11_la_SOURCES = \
	collector.pb.cc \
	lightstep_carrier.pb.cc

# Note: To set the protobuf library include and library flags, e.g.,
#
#   ./configure protobuf_CFLAGS="..." protobuf_LIBS="..."
liblightstep_core_cxx11_la_LIBADD   =
liblightstep_core_cxx11_la_CXXFLAGS = $(AM_CXXFLAGS) $(protobuf_CFLAGS) -I$(builddir)
liblightstep_core_cxx11_la_LDFLAGS  = $(protobuf_LIBS)

# Options
AM_CXXFLAGS = -Wno-deprecated-declarations

EXTRA_DIST = \
	mapbox_variant/LICENSE

# The builtin gRPC transport is built by default.  When supplying your
# own gRPC transport, you can disable the built-in recorder:
#
#   ./configure --disable-grpc
if ENABLE_GRPC

GRPC_PROTO_SRC = \
	$(proto_srcdir)/collector.proto

GRPC_PROTO_GEN = \
	collector.grpc.pb.h \
	collector.grpc.pb.cc

BUILT_SOURCES += $(GRPC_PROTO_GEN)
CLEANFILES    += $(GRPC_PROTO_GEN) proto.grpc.stamp

# TODO make grpc_cpp_plugin a configure option
#
proto.grpc.stamp: $(GRPC_PROTO_SRC)
	$(PROTOC) --proto_path=$(proto_srcdir) --grpc_out=$(builddir) \
		--plugin=protoc-gen-grpc=`which grpc_cpp_plugin` \
		$(proto_srcdir)/collector.proto
	touch proto.grpc.stamp

$(GRPC_PROTO_GEN): proto.grpc.stamp

# LightStep gRPC recorder library
lib_LTLIBRARIES += liblightstep_grpc_cxx11.la

liblightstep_grpc_cxx11_la_SOURCES = \
	recorder.cc

nobase_nodist_include_HEADERS += \
	collector.grpc.pb.h

nodist_liblightstep_grpc_cxx11_la_SOURCES = \
	collector.grpc.pb.cc

# Note $(grpc_LIBS)++ shenanigans. The grpc C++ pkg-config input (grpc++.pc) uses
# characters that are invalid to autoconf, so we can't find it.  Instead, we locate
# the C library and tack on a '++'.
liblightstep_grpc_cxx11_la_LIBADD   = liblightstep_core_cxx11.la
liblightstep_grpc_cxx11_la_CXXFLAGS = $(AM_CXXFLAGS) $(grpc_CFLAGS) $(protobuf_CFLAGS) -I$(builddir)
liblightstep_grpc_cxx11_la_LDFLAGS  = $(protobuf_LIBS) $(grpc_LIBS)++
endif
