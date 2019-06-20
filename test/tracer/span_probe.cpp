#include <cassert>
#include <iostream>
#include <sstream>
#include <string>

#include <opentracing/dynamic_load.h>

int main(int argc, char* argv[]) {
  if (argc != 4) {
    std::cerr << "Usage: <tracer_library> <host> <port>\n";
    return -1;
  }

  // Load the tracer library.
  std::string error_message;
  auto handle_maybe =
      opentracing::DynamicallyLoadTracingLibrary(argv[1], error_message);
  if (!handle_maybe) {
    std::cerr << "Failed to load tracer library " << error_message << "\n";
    return -1;
  }

  // Generate the config
  auto host = argv[2];
  auto port = argv[3];
  std::ostringstream oss;
  oss << R"({
      "component_name" : "span_probe",
      "access_token": "abc123",
      "collector_plaintext": true,
      "use_stream_recorder": true,
      "satellite_endpoints": [{"host": ")";
  oss << host << R"(", "port": )" << port;
  oss << R"(}]})";

  // Construct a tracer.
  auto& tracer_factory = handle_maybe->tracer_factory();
  auto tracer_maybe =
      tracer_factory.MakeTracer(oss.str().c_str(), error_message);
  if (!tracer_maybe) {
    std::cerr << "Failed to create tracer " << error_message << "\n";
    return -1;
  }
  auto& tracer = *tracer_maybe;

  // Use the tracer to create some spans.
  tracer->StartSpan("A");

  tracer->Close();
  return 0;
}
