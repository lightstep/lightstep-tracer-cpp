#!/bin/bash
# This script runs from the vendor directory.
set -e

CMAKE=${CMAKE:-cmake}
BUILD_DIR=`pwd` # i.e., <repo>/vendor
DOWNLOADS="/tmp/lightstep_downloads"

CPP_NETLIB_DIR=${BUILD_DIR}/cpp-netlib
CPP_NETLIB_GITHUB="https://github.com/cpp-netlib/cpp-netlib.git"
CPP_NETLIB_BRANCH="origin/0.13-release"

BOOST_SITE="http://sourceforge.net/projects/boost/files/boost/1.60.0"
BOOST_URL=${BOOST_SITE}/boost_1_60_0.tar.gz
BOOST_TMP=${DOWNLOADS}/`basename ${BOOST_URL}`
BOOST_BASE=`basename -s .tar.gz ${BOOST_URL}`
BOOST_DIR=${BUILD_DIR}/${BOOST_BASE}
BOOST_ROOT=${BOOST_DIR}/build

# Note: This script is mostly designed to run in a Docker container
# running Linux. It would work on OS X except that the path to OpenSSL
# and needs to be externally provided.
function build_boost()
{
    mkdir -p ${DOWNLOADS}
    chmod 0777 ${DOWNLOADS}
    if [ ! -f ${BOOST_TMP} ]; then
	wget -O ${BOOST_TMP} ${BOOST_URL}
    fi

    tar -xzf ${BOOST_TMP}
    cd ${BOOST_DIR}
    mkdir -p build

    ./bootstrap.sh --prefix=${BOOST_DIR}/build
    ./b2 --without-python -s NO_BZIP2=1 link=static install

    BOOST_CPPFLAGS="-I/${BOOST_ROOT}/include"
    BOOST_LDFLAGS="-I/${BOOST_ROOT}/lib"

    cd ..
}

function build_netlib()
{
    if [ ! -d ${CPP_NETLIB_DIR} ]; then
	cd ${BUILD_DIR}
	rm -rf cpp-netlib
	git clone ${CPP_NETLIB_GITHUB}
    fi

    cd ${CPP_NETLIB_DIR}
    git checkout ${CPP_NETLIB_BRANCH}

    git submodule init
    git submodule update

    mkdir -p ./build
    cd build
    ${CMAKE} \
	-DBOOST_ROOT="${BOOST_ROOT}" \
	-DCMAKE_CXX_FLAGS="-std=c++11 ${CXXFLAGS}" \
	..
    make all -j2

    # 0.13-release tests apparently hang when running in Docker.
    # make test

    cd ..
}

build_boost
build_netlib
