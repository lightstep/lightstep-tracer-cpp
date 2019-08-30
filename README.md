# lightstep-tracer-cpp
[![MIT license](http://img.shields.io/badge/license-MIT-blue.svg)](http://opensource.org/licenses/MIT)

The LightStep distributed tracing library for C++.

## Installation

The library supports being built in several configurations to support
a variety of uses.  There are three transport options available:

1. Streaming HTTP transport (*NEW in 0.9.x*): This option uses multiple outbound HTTP 1.1 connections to send spans to LightStep.  This transport option is optimized for concurrency and throughput, compared with the gRPC transport option. TLS is not currently supported in this configuration.
1. gRPC transport (*DEFAULT*): This option uses the gRPC library for transport. This transport option is not optimized for concurrency and throughput, compared with the the Streaming HTTP transport option.
1. User-defined transport: This option allows the user to supply custom logic for transporting span objects to LightStep. Users must implement one of the LightStep-supported transports themselves with this option.

The library also supports dynamic loading, for applications that support more than one OpenTracing-compatible tracer.  To use the dynamic library, we recommend installing the binary plugin included with each release (e.g., [the 0.9.0 plugin](https://github.com/lightstep/lightstep-tracer-cpp/releases/download/v0.9.0/linux-amd64-liblightstep_tracer_plugin.so.gz)).

### Requirements

To build and install the LightStep distributed tracing library, you will need to have several tools and libraries intalled.

1. cmake
1. protobuf
1. grpc (for gRPC transport)
1. c-ares (for Streaming HTTP transport)
1. libevent (for Streaming HTTP transport)
1. [OpenTracing C++](https://github.com/opentracing/opentracing-cpp) library.

### Building

Get and install the current [1.5.x release of OpenTracing](https://github.com/opentracing/opentracing-cpp/archive/v1.5.1.tar.gz) as described in that repository's README.  After installing the OpenTracing APIs, generate the Makefile with the desired build options.

1. For gRPC, use `-DWITH_GRPC=ON`
1. For Streaming HTTP, use `-DWITH_LIBEVENT=ON -DWITH_CARES=ON -DWITH_GRPC=OFF`
1. For Dynamic loading support, add `-DWITH_DYNAMIC_LOAD=ON`

Run these commands to configure and build the package.

```
$ mkdir .build
$ cd .build
$ cmake ..
$ make
$ sudo make install
```

### OS X specific steps

Several packages are required to complete this build.  To install all the dependencies on OS X using `brew`:

```
brew install cmake
brew install protobuf
brew install grpc         # for gRPC
brew install c-ares       # for Streaming HTTP
brew install libevent     # for Streaming HTTP
brew install pkg-config
```

### Windows specific steps

Basic support is available for windows. Dependencies can be installed with [vcpkg](https://github.com/microsoft/vcpkg).

```
vcpkg install libevent:x64-windows-static
vcpkg install protobuf:x64-windows-static
vcpkg install opentracing:x64-windows-static
vcpkg install c-ares:x64-windows-static
```

The lightstep tracer can then be built with

```
$VCPKG_DIR=<path to where vcpkg was installed>
cmake -DBUILD_SHARED_LIBS=OFF `
      -DWITH_DYNAMIC_LOAD=OFF `
      -DWITH_GRPC=OFF `
      -DWITH_LIBEVENT=ON `
      -DWITH_CARES=ON `
      -DVCPKG_TARGET_TRIPLET=x64-windows-static `
      "-DCMAKE_TOOLCHAIN_FILE=$VCPKG_DIR\scripts\buildsystems\vcpkg.cmake" `
      <path-to-lightstep>
cmake --build .
```

The [streaming example](example/stream/main.cpp) can  be run with `.\example\stream\Debug\stream.exe`.

## Getting started

To initialize the LightStep tracer, configure the options and
construct the object as shown below.  The `Tracer` returned by
`lightstep::MakeLightStepTracer` may be passed manually through the
application, or it can be set as the `opentracing::Global()` tracer. 

```cpp
#include <opentracing/tracer.h>
#include <lightstep/tracer.h>

const bool use_streaming_tracer = true;

void initGlobalTracer() {
  lightstep::LightStepTracerOptions options;
  options.component_name = "c++ quickstart app";
  options.access_token = "hello";

  // Configure the tracer to send to the local developer satellite:
  options.collector_plaintext = true;
  if (use_streaming_tracer) {
    options.satellite_endpoints = {{"localhost", 8360}};
    options.use_stream_recorder = true;
  } else {
    options.collector_host = "localhost";
    options.collector_port = 8360;
    options.use_stream_recorder = false;
  }

  auto tracer = lightstep::MakeLightStepTracer(std::move(options));

  opentracing::Tracer::InitGlobal(tracer);
}

int main() {
  initGlobalTracer();

  auto span = opentracing::Tracer::Global()->StartSpan("demo");

  span->SetTag("key", "value");
  span->Log({{"logkey", "logval"},
             {"number", 1}});
  span->Finish();

  opentracing::Tracer::Global()->Close();
  
  return 0;
}
```

For instrumentation documentation, see the [opentracing-cpp docs](https://github.com/opentracing/opentracing-cpp).

## Dynamic loading

The LightStep tracer supports dynamic loading and construction from a JSON configuration. See the [schema](lightstep-tracer-configuration/tracer_configuration.schema.json) for details on the JSON format.

## Testing

The lightstep-streaming Python module, which is contained in this repo, is performance tested in CI using [LightStep Benchmarks](https://github.com/lightstep/lightstep-benchmarks). Performance regression tests are run automatically every commit, and performance graphs can be generated with manual approval. **This repo will show a yellow dot for CI test status even when all of the automatic tests have run. Because LightStep Benchmarks performance graphs are only generated after manual approval and CircleCI counts them as "running" before they've been approved, you won't see a green status check mark unless you've manually approved performance graph generation.**
