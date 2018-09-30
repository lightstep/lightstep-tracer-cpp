#!/bin/bash

set -e
apt-get update 
apt-get install --no-install-recommends --no-install-suggests -y \
                build-essential \
                cmake \
                pkg-config \
                git \
                ca-certificates \
                automake \
                autogen \
                autoconf \
                libtool \
                gnupg2 \
                ssh \
                libgflags2.2 libgflags-dev \
                python python-setuptools python-pip
pip install gcovr
