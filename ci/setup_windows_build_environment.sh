#!/bin/bash

set -e
cd $HOMEDRIVE

# install cmake
curl -O https://github.com/Kitware/CMake/releases/download/v3.15.1/cmake-3.15.1-win64-x64.zip

# intall visual studio
curl -O https://aka.ms/vs/15/release/vs_community.exe
./vs_community.exe

git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.bat
./vcpkg integrate install
./vcpkg install libevent:x64-windows-static
./vcpkg install protobuf:x64-windows-static
./vcpkg install opentracing:x64-windows-static
./vcpkg install cares:x64-windows-static
