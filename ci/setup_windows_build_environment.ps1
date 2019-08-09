$ErrorActionPreference = "Stop"

git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.bat
./vcpkg integrate install
./vcpkg install libevent:x64-windows-static
./vcpkg install protobuf:x64-windows-static
./vcpkg install opentracing:x64-windows-static
./vcpkg install c-ares:x64-windows-static
