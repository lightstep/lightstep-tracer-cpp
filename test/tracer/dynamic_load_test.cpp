#include "opentracing/dynamic_load.h"

#include <sstream>

#include "test/mock_satellite/mock_satellite_handle.h"
#include "test/ports.h"
#include "test/utility.h"

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

TEST_CASE("dynamic_load") {
  std::string error_message;

  auto handle_maybe = opentracing::DynamicallyLoadTracingLibrary(
      "./liblightstep_tracer_plugin.so", error_message);
  INFO(error_message);
  REQUIRE(error_message.empty());
  REQUIRE(handle_maybe);
  auto& tracer_factory = handle_maybe->tracer_factory();

  SECTION("Creating a tracer with invalid json gives an error.") {
    const char* config = R"(
    {
      "component_name": "xyz"
    ]
    )";
    auto tracer_maybe = tracer_factory.MakeTracer(config, error_message);
    INFO(error_message);
    CHECK(!error_message.empty());
    CHECK(!tracer_maybe);
  }

  SECTION(
      "A tracer and spans can be created from the dynamically loaded "
      "library.") {
    std::unique_ptr<MockSatelliteHandle> mock_satellite{new MockSatelliteHandle{
        static_cast<uint16_t>(PortAssignments::DynamicLoadTest)}};

    // Create a tracer.
    std::ostringstream oss;
    oss << R"({
      "component_name" : "dynamic_load_test",
      "access_token": "abc123",
      "collector_plaintext": true,
      "use_stream_recorder": true,
      "satellite_endpoints": [ {"host": "0.0.0.0", "port": 
    )";
    oss << static_cast<int>(PortAssignments::DynamicLoadTest);
    oss << R"(
      }]
    })";

    auto tracer_maybe =
        tracer_factory.MakeTracer(oss.str().c_str(), error_message);
    REQUIRE(error_message.empty());
    REQUIRE(tracer_maybe);
    auto& tracer = *tracer_maybe;

    auto span = tracer->StartSpan("abc");
    span->Finish();
    tracer->Close();
    REQUIRE(
        IsEventuallyTrue([&] { return mock_satellite->spans().size() == 1; }));
  }
}
