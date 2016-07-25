#!/bin/bash
# This script runs from the vendor directory.
set -e

BDIR=`pwd` # i.e., <repo>/vendor
BRANCH="origin/0.13-release"
CPP_NETLIB="${BDIR}/cpp-netlib"

if [ ! -d ${CPP_NETLIB} ]; then
    rm -rf cpp-netlib
    git clone git@github.com:cpp-netlib/cpp-netlib.git    
fi

cd ${CPP_NETLIB}
git checkout ${BRANCH}

git submodule init
git submodule update

mkdir -p ./build
cd build
${CMAKE} \
  -DBOOST_INCLUDEDIR="${BOOST_CPPFLAGS}" \
  -DBOOST_LIBRARYDIR="${BOOST_LDFLAGS}" \
  -DCMAKE_CXX_FLAGS="-std=c++11 ${CXXFLAGS}" \
  ..
make all -j2
make test
cd ..
