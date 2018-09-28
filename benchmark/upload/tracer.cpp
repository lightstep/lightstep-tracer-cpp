#include "tracer.h"

#include <exception>
#include <iostream>
#include <string>

#include "../../src/lightstep_tracer_factory.h"

#include <google/protobuf/util/json_util.h>

namespace lightstep {
//------------------------------------------------------------------------------
// MakeTracer
//------------------------------------------------------------------------------
std::shared_ptr<opentracing::Tracer> MakeTracer(
    const configuration_proto::TracerConfiguration& configuration) {
  // Reserialize the tracer configuration to Json so that we can reuse the
  // existing TracerFactory functionality. We don't care too much about
  // performance here.
  std::string json;
  auto was_successful =
      google::protobuf::util::MessageToJsonString(configuration, &json);
  if (!was_successful.ok()) {
    std::cerr << "Failed to serialize tracer configuration: "
              << was_successful.ToString() << "\n";
    std::terminate();
  }
  LightStepTracerFactory tracer_factory;
  std::string error_message;
  auto tracer_maybe = tracer_factory.MakeTracer(json.c_str(), error_message);
  if (!tracer_maybe) {
    std::cerr << "Failed to construct tracer: " << error_message << "\n";
    std::terminate();
  }
  return *tracer_maybe;
}
}  // namespace lightstep
