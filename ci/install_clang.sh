#!/bin/bash

set -e

apt-get install --no-install-recommends --no-install-suggests -y \
                gnupg2 \
                software-properties-common \
                wget

wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - 
apt-add-repository "deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-6.0 main" 
apt-get update 
apt-get install -y clang-6.0 clang-tidy
