PROTO_SRC = ../../lightstep-tracer-common
PROTO_OUT = proto

$(PROTO_OUT)/%.pb.cc $(PROTO_OUT)/%.pb.h: $(PROTO_SRC)/%.proto
	mkdir -p $(PROTO_OUT)
	protoc --proto_path=$(PROTO_SRC) --cpp_out=$(PROTO_OUT) $<

lib_LTLIBRARIES = liblightstep_core_cxx11.la

liblightstep_core_cxx11_la_SOURCES = \
	impl.cc \
	span.cc \
	tracer.cc \
	util.cc \
	proto/collector.pb.cc

liblightstep_core_cxx11_la_LIBADD   = $(protobuf_LIBS)
liblightstep_core_cxx11_la_CXXFLAGS = $(AM_CXXFLAGS) $(protobuf_CFLAGS)
liblightstep_core_cxx11_la_LDFLAGS  =

if ENABLE_GRPC
lib_LTLIBRARIES += liblightstep_grpc_cxx11.la

liblightstep_grpc_cxx11_la_SOURCES = \
	recorder.cc

liblightstep_grpc_cxx11_la_LIBADD   = liblightstep_core_cxx11.la
liblightstep_grpc_cxx11_la_CXXFLAGS = $(AM_CXXFLAGS)
liblightstep_grpc_cxx11_la_LDFLAGS  =
endif
