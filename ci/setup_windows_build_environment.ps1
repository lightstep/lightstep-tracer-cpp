$ErrorActionPreference = "Stop"
trap { $host.SetShouldExit(1) }

git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
$VCPKG_DIR=(Get-Item -Path ".\").FullName
./bootstrap-vcpkg.bat
./vcpkg integrate install
./vcpkg install libevent:x64-windows
./vcpkg install protobuf:x64-windows
./vcpkg install opentracing:x64-windows
./vcpkg install c-ares:x64-windows
