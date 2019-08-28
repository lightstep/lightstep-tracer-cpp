$ErrorActionPreference = "Stop";
trap { $host.SetShouldExit(1) }

$action = $args[0]

$SRC_DIR=(Get-Item -Path ".\").FullName
mkdir build
$BUILD_DIR="$SRC_DIR\build"
$VCPKG_DIR="$SRC_DIR\vcpkg"

switch ($action) {
	"build" {
		cd "$BUILD_DIR"
		cmake $SRC_DIR `
          -DVCPKG_TARGET_TRIPLET=x64-windows-static `
			    "-DCMAKE_TOOLCHAIN_FILE=$VCPKG_DIR\scripts\buildsystems\vcpkg.cmake" `
          -DBUILD_SHARED_LIBS=OFF `
					-DWITH_DYNAMIC_LOAD=OFF `
					-DWITH_GRPC=OFF `
					-DWITH_LIBEVENT=ON
    $exit = $LASTEXITCODE
    if ($exit -ne 0) {
      exit $exit
    }
		cmake --build .
    $exit = $LASTEXITCODE
    if ($exit -ne 0) {
      exit $exit
    }
	}
	default {
		echo "unknown action: $action"
		exit 1
	}
}
