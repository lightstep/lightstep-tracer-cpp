# TODO(jmacd): Replace the proto-generation rule with one using the protoc docker image.
PROTOC = protoc
PROTO_SRC = ../../lightstep-tracer-common
PROTO_OUT = proto

$(PROTO_OUT)/%.pb.cc $(PROTO_OUT)/%.pb.h: $(PROTO_SRC)/%.proto
	mkdir -p $(PROTO_OUT)
	$(PROTOC) --proto_path=$(PROTO_SRC) --grpc_out=$(PROTO_OUT) --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` $<
	$(PROTOC) --proto_path=$(PROTO_SRC) --cpp_out=$(PROTO_OUT) $<

lib_LTLIBRARIES = liblightstep_core_cxx11.la

liblightstep_core_cxx11_la_SOURCES = \
	impl.cc \
	span.cc \
	tracer.cc \
	util.cc \
	proto/collector.pb.cc

liblightstep_core_cxx11_la_LIBADD   =
liblightstep_core_cxx11_la_CXXFLAGS = $(AM_CXXFLAGS) $(protobuf_CFLAGS)
liblightstep_core_cxx11_la_LDFLAGS  = $(protobuf_LIBS)

if ENABLE_GRPC
lib_LTLIBRARIES += liblightstep_grpc_cxx11.la

liblightstep_grpc_cxx11_la_SOURCES = \
	recorder.cc \
	proto/collector.grpc.pb.cc

# Note $(grpc_LIBS)++ shenanigans. The grpc C++ pkg-config input (grpc++.pc) uses
# characters that are invalid to autoconf, so we can't find it.  Instead, we locate
# the C library and tack on a '++'.
liblightstep_grpc_cxx11_la_LIBADD   = liblightstep_core_cxx11.la
liblightstep_grpc_cxx11_la_CXXFLAGS = $(AM_CXXFLAGS) $(grpc_CFLAGS) $(protobuf_CFLAGS)
liblightstep_grpc_cxx11_la_LDFLAGS  = $(protobuf_LIBS) $(grpc_LIBS)++
endif
