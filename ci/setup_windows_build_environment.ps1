$ErrorActionPreference = "Stop"
trap { $host.SetShouldExit(1) }

git clone -b 2020.01 https://github.com/Microsoft/vcpkg.git
cd vcpkg
$VCPKG_DIR=(Get-Item -Path ".\").FullName
./bootstrap-vcpkg.bat
./vcpkg integrate install
./vcpkg install libevent:x64-windows-static
./vcpkg install protobuf:x64-windows-static
./vcpkg install opentracing:x64-windows-static
./vcpkg install c-ares:x64-windows-static
