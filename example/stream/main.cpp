#include <lightstep/tracer.h>
#include <cassert>
#include <chrono>
#include <cstdlib>  // for std::getenv
#include <iostream>
#include <string>
#include <thread>

static void MakeSpan(opentracing::Tracer& tracer, int span_index) {
  std::cout << "Make Span: " << span_index << "\n";
  auto span = tracer.StartSpan("span_" + std::to_string(span_index));
  assert(span != nullptr);
  for (int tag_index = 0; tag_index < 25; ++tag_index) {
    span->SetTag("tag_" + std::to_string(tag_index), tag_index);
  }
}

int main() {
  lightstep::LightStepTracerOptions options;
  // See https://docs.lightstep.com/docs/developer-mode for instructions on
  // setting up a LightStep developer satellite on localhost:8360.
  options.collector_plaintext = true;
  options.satellite_endpoints = {{"localhost", 8360}};
  options.use_stream_recorder = true;
  options.verbose = true;
  options.component_name = "Stream";
  if (const char* access_token = std::getenv("LIGHTSTEP_ACCESS_TOKEN")) {
    options.access_token = access_token;
  } else {
    std::cerr << "You must set the environmental variable "
                 "`LIGHTSTEP_ACCESS_TOKEN` to your access token!\n";
    return -1;
  }

  auto tracer = MakeLightStepTracer(std::move(options));
  assert(tracer != nullptr);
  for (int i = 0; i < 1000; ++i) {
    MakeSpan(*tracer, i);
  }
  std::cout << "Closing tracer\n";
  tracer->Close();
  return 0;
}
