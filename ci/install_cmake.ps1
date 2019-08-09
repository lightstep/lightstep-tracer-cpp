# See https://stackoverflow.com/a/9949909/4447365
$ErrorActionPreference = "Stop"

$CMAKE_VERSION=3.15.2
$CWD=(Get-Item -Path ".\").FullName
(new-object System.Net.WebClient).DownloadFile('https://github.com/Kitware/CMake/releases/download/v$CMAKE_VERSION/cmake-$CMAKE_VERSION-win64-x64.zip', $CWD + '\cmake-$CMAKE_VERSION-win64-x64.zip')

unzip cmake-$CMAKE_VERSION-win64-x64.zip

$ENV:PATH="$ENV:PATH;$CWD\cmake-$CMAKE_VERSION-win64-x64\bin"
cmake --help
