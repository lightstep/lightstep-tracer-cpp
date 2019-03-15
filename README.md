# lightstep-tracer-cpp
[![MIT license](http://img.shields.io/badge/license-MIT-blue.svg)](http://opensource.org/licenses/MIT)

The LightStep distributed tracing library for C++.

## Installation

The library supports being built in several configurations to support a variety of uses.  The default build configuration includes gRPC support for sending data to LightStep, or alternately the library may be configured without gRPC to use a user-defined transport library.

The library also supports dynamic loading, for applications that support more than one OpenTracing-compatible tracer.  To use the dynamic library, we recommend installing the binary plugin included with each release (e.g., [the 0.8.1 plugin](https://github.com/lightstep/lightstep-tracer-cpp/releases/download/v0.8.1/linux-amd64-liblightstep_tracer_plugin.so.gz)).

### Requirements

To build and install the LightStep distributed tracing library, you will need to have several tools and libraries intalled.

1. cmake
1. protobuf
1. grpc

This library also depends on the current release of the [OpenTracing C++](https://github.com/opentracing/opentracing-cpp) API.

### Building

Get and install the current [1.5.x release of OpenTracing](https://github.com/opentracing/opentracing-cpp/archive/v1.5.1.tar.gz).  (The same sequence of commands printed below may be used to install OpenTracing library.)  After installing the OpenTracing APIs, run these commands to configure and build the package.

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
brew install grpc
brew install pkg-config
```

## Getting started

To initialize the LightStep library in particular, either retain a reference to the LightStep `opentracing::Tracer` implementation and/or set the global `Tracer` like so:

```cpp
#include <opentracing/tracer.h>
#include <lightstep/tracer.h>

void initGlobalTracer() {
  lightstep::LightStepTracerOptions options;
  options.component_name = "c++ quickstart app";
  options.access_token = "hello";

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
