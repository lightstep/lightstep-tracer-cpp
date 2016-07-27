
lib_LTLIBRARIES = liblightstep_nonetwork_cxx11.la

liblightstep_nonetwork_cxx11_la_SOURCES = \
	impl.cc \
	span.cc \
	tracer.cc \
	util.cc \
	dropbox_json/json11.cpp

# TODO these Boost dependencies for the 'nonetwork' are not a good thing,
# they are only here to escape and unescape URIs.  Separate the pure, lightweight
# library from the HTTP header stuff somehow?
BLOATYCPPFLAGS = -I$(top_srcdir)/vendor/cpp-netlib \
		-I$(top_srcdir)/vendor/boost_1_60_0/build/include
BLOATYLDFLAGS = -L$(top_srcdir)/vendor/cpp-netlib/build/libs/network/src \
		-L$(top_srcdir)/vendor/boost_1_60_0/build/lib \
		-lcppnetlib-uri

liblightstep_nonetwork_cxx11_la_CXXFLAGS = $(AM_CXXFLAGS) -I dropbox_json $(BLOATYCPPFLAGS)
liblightstep_nonetwork_cxx11_la_LIBADD   =
liblightstep_nonetwork_cxx11_la_LDFLAGS  = $(BLOATYLDFLAGS)

if ENABLE_CPP_NETLIB
liblightstep_cxx11_la_SOURCES = \
	recorder.cc

liblightstep_cxx11_la_LIBADD   = liblightstep_nonetwork_cxx11.la
liblightstep_cxx11_la_CXXFLAGS = $(AM_CXXFLAGS) \
				-I$(top_srcdir)/vendor/cpp-netlib \
				-I$(top_srcdir)/vendor/boost_1_60_0/build/include \
				-I dropbox_json
liblightstep_cxx11_la_LDFLAGS  = -L$(top_srcdir)/vendor/cpp-netlib/build/libs/network/src \
				-L$(top_srcdir)/vendor/boost_1_60_0/build/lib \
				-lboost_system \
				-lcppnetlib-uri

lib_LTLIBRARIES += liblightstep_cxx11.la

if ENABLE_OPENSSL
liblightstep_cxx11_la_CXXFLAGS += $(openssl_CFLAGS)
liblightstep_cxx11_la_LIBADD   += $(openssl_LIBS)
endif

endif
