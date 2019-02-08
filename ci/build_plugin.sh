#!/bin/bash

set -e

[ -z "${OPENTRACING_VERSION}" ] && export OPENTRACING_VERSION="v1.5.0"
[ -z "${GRPC_VERSION}" ] && export GRPC_VERSION="v1.10.0"
[ -z "${LIBEVENT_VERSION}" ] && export LIBEVENT_VERSION="v2.1.8"
[ -z "${CARES_VERSION}" ] && export CARES_VERSION="1.15.0"

# Compile for a portable cpu architecture
export CFLAGS="-march=x86-64 -fPIC"
export CXXFLAGS="-march=x86-64 -fPIC"
export LDFLAGS="-fPIC"

### Build C-ares
cd "${BUILD_DIR}"
wget https://github.com/c-ares/c-ares/releases/download/cares-${CARES_VERSION//./_}/c-ares-${CARES_VERSION}.tar.gz
tar zxf c-ares-${CARES_VERSION}.tar.gz
cd c-ares-${CARES_VERSION}
./configure --with-pic=yes \
            --enable-shared=no \
            --enable-static=yes

### Build gRPC
cd "${BUILD_DIR}"
git clone -b ${GRPC_VERSION} https://github.com/grpc/grpc
cd grpc
git submodule update --init
make HAS_SYSTEM_CARES=true HAS_SYSTEM_PROTOBUF=false static
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

# Build libevent
cd "${BUILD_DIR}"
git clone -b release-${LIBEVENT_VERSION/v/}-stable https://github.com/libevent/libevent
cd libevent
./autogen.sh
./configure \
    --enable-shared=no \
    --enable-static=yes \
    --with-pic \
    --disable-openssl
make
make install

# Build OpenTracing
cd "${BUILD_DIR}"
git clone -b ${OPENTRACING_VERSION} https://github.com/opentracing/opentracing-cpp.git
cd opentracing-cpp
mkdir .build && cd .build
cmake -DCMAKE_BUILD_TYPE=Release \
      -DBUILD_SHARED_LIBS=OFF \
      -DBUILD_TESTING=OFF \
      -DBUILD_MOCKTRACER=OFF \
      ..
make VERBOSE=1 && make install

# Build LightStep
cd "${BUILD_DIR}"
mkdir lightstep-tracer-cpp && cd lightstep-tracer-cpp
cat <<EOF > export.map
{
  global:
    OpenTracingMakeTracerFactory;
  local: *;
};
EOF
cmake -DCMAKE_BUILD_TYPE=Release \
      -DBUILD_TESTING=OFF \
      -DBUILD_STATIC_LIBS=OFF \
      -DWITH_LIBEVENT=ON \
      -DWITH_CARES=ON \
      -DCMAKE_SHARED_LINKER_FLAGS="-static-libstdc++ -static-libgcc -Wl,--version-script=${PWD}/export.map" \
      -DDEFAULT_SSL_ROOTS_PEM:STRING=${BUILD_DIR}/grpc/etc/roots.pem \
      "${SRC_DIR}"

make VERBOSE=1 && make install
cp /usr/local/lib/liblightstep_tracer.so /liblightstep_tracer_plugin.so
