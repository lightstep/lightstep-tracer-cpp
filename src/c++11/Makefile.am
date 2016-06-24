
lib_LTLIBRARIES = liblightstep_basic_cxx11.la

liblightstep_basic_cxx11_la_SOURCES = \
	impl.cc \
	span.cc \
	tracer.cc \
	util.cc \
	dropbox_json/json11.cpp

liblightstep_basic_cxx11_la_CXXFLAGS = $(AM_CXXFLAGS) -I dropbox_json
liblightstep_basic_cxx11_la_LIBADD   =

if ENABLE_CPP_NETLIB
liblightstep_cxx11_la_SOURCES = \
	recorder.cc

liblightstep_cxx11_la_CXXFLAGS = $(AM_CXXFLAGS) $(CPP_NETLIB_CPPFLAGS) -I dropbox_json
liblightstep_cxx11_la_LIBADD   = liblightstep_basic_cxx11.la $(CPP_NETLIB_LDFLAGS)

lib_LTLIBRARIES += liblightstep_cxx11.la

if ENABLE_OPENSSL
liblightstep_cxx11_la_CXXFLAGS += $(openssl_CFLAGS)
liblightstep_cxx11_la_LIBADD   += $(openssl_LIBS)
endif

endif

