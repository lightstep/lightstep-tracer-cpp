#!/bin/bash

set -e

[ -z "${OPENTRACING_VERSION}" ] && export OPENTRACING_VERSION="v1.3.0"
[ -z "${GRPC_VERSION}" ] && export GRPC_VERSION="v1.10.0"

# Compile for a portable cpu architecture
export CFLAGS="-march=x86-64 -fPIC"
export CXXFLAGS="-march=x86-64 -fPIC"
export LDFLAGS="-fPIC"

### Build gRPC
cd "${BUILD_DIR}"
git clone -b ${GRPC_VERSION} https://github.com/grpc/grpc
cd grpc
git submodule update --init
make HAS_SYSTEM_PROTOBUF=false static
# According to https://github.com/grpc/grpc/issues/7917#issuecomment-243800503
# make install won't work with the static target, so manually copy in the files
# needed.
cp -r include/grpc /usr/local/include/
cp -r include/grpc++ /usr/local/include/
cp -r include/grpcpp /usr/local/include/
cp libs/opt/*.a /usr/local/lib
cp libs/opt/protobuf/*.a /usr/local/lib
cp bins/opt/grpc_cpp_plugin /usr/local/bin
mkdir -p /usr/local/lib/pkgconfig
cp libs/opt/pkgconfig/*.pc /usr/local/lib/pkgconfig
cd third_party/protobuf
make install

# Build OpenTracing
cd "${BUILD_DIR}"
git clone -b ${OPENTRACING_VERSION} https://github.com/opentracing/opentracing-cpp.git
cd opentracing-cpp
mkdir .build && cd .build
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_CXX_FLAGS="-fPIC" \
      -DBUILD_SHARED_LIBS=OFF \
      -DBUILD_TESTING=OFF \
      -DBUILD_MOCKTRACER=OFF \
      ..
make && make install

# Build LightStep
cd "${BUILD_DIR}"
mkdir lightstep-tracer-cpp && cd lightstep-tracer-cpp
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_CXX_FLAGS="-fPIC" \
      -DBUILD_TESTING=OFF \
      "${SRC_DIR}"
make && make install

# Create a plugin
cd "${BUILD_DIR}"
mkdir lightstep-tracer-plugin && cd lightstep-tracer-plugin
cat <<EOF > export.map
{
  global:
    OpenTracingMakeTracerFactory;
  local: *;
};
EOF
cat <<EOF > Makefile
all:
	gcc -shared -o liblightstep_tracer_plugin.so \
      -Wl,--version-script=export.map \
			-L/usr/local/lib \
			-Wl,--whole-archive \
			/usr/local/lib/liblightstep_tracer.a \
			-Wl,--no-whole-archive \
      /usr/local/lib/libopentracing.a \
			/usr/local/lib/libprotobuf.a \
			/usr/local/lib/libgrpc.a \
			/usr/local/lib/libgrpc++.a \
      -static-libstdc++ -static-libgcc
EOF
make
cp liblightstep_tracer_plugin.so /
