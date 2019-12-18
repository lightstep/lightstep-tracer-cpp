#!/bin/bash

set -e
apt-get update 
apt-get install --no-install-recommends --no-install-suggests -y \
                build-essential \
                cmake \
                pkg-config \
                git \
                ca-certificates \
                curl \
                automake \
                autogen \
                autoconf \
                libtool \
                gnupg2 \
                ssh \
                lcov \
                vim \
                gdb \
                wget \
                python python-setuptools python-pip \
                zlib1g-dev \
                libffi-dev \
                python3 \
                zip \
                gettext-base
pip install wheel
