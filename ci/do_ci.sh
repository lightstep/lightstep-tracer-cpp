#!/bin/bash

set -e

[ -z "${SRC_DIR}" ] && export SRC_DIR="`pwd`"
[ -z "${BUILD_DIR}" ] && export BUILD_DIR="`mktemp -d`"

if [[ "$1" == "cmake.test" ]]; then
  cd "${BUILD_DIR}"
  cmake -DCMAKE_BUILD_TYPE=Debug "${SRC_DIR}"
  make
  make test
  exit 0
fi
