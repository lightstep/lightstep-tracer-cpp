#include "tracer/json_options.h"

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

TEST_CASE("LightStepTracerFactory") {
  std::string error_message;

  SECTION("We can specify propagation modes via a tracers json configuration") {
    const char* config = R"({
      "component_name": "t",
      "propagation_modes": ["lightstep", "b3", "envoy", "trace_context"]
    })";
    auto options_maybe = MakeTracerOptions(config, error_message);
    REQUIRE(options_maybe);
    std::vector<PropagationMode> expected_propagation_modes = {
        PropagationMode::lightstep, PropagationMode::b3, PropagationMode::envoy,
        PropagationMode::trace_context};
    REQUIRE(options_maybe->propagation_modes == expected_propagation_modes);
  }

  SECTION("MakeTracerOptions fails if there's an invalid propagation_mode") {
    const char* config = R"({
      "component_name": "t",
      "propagation_modes": ["abc"]
    })";
    auto options_maybe = MakeTracerOptions(config, error_message);
    REQUIRE(!options_maybe);
  }
}
