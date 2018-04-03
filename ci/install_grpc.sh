#!/bin/bash

set -e

[ -z "${GRPC_VERSION}" ] && export GRPC_VERSION="v1.10.0"

apt-get update 
apt-get install --no-install-recommends --no-install-suggests -y \
              golang
                  


### Build gRPC
cd /
git clone -b ${GRPC_VERSION} https://github.com/grpc/grpc
cd grpc
git submodule update --init
make HAS_SYSTEM_PROTOBUF=false && make install
make && make install
cd third_party/protobuf
make install
ldconfig
