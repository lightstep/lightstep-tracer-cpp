#include "opentracing/dynamic_load.h"

#include <sstream>

#include "test/mock_satellite/mock_satellite_handle.h"
#include "test/ports.h"
#include "test/utility.h"

#include "3rd_party/catch2/catch.hpp"

/////////////////////////////////////////////////////////
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}
/////////////////////////////////////////////////////////

using namespace lightstep;

TEST_CASE("dynamic_load") {
  std::string error_message;

  auto handle_maybe = opentracing::DynamicallyLoadTracingLibrary(
      "liblightstep_tracer_plugin.so", error_message);
  INFO("TEST_TEMP_DIR: " << exec("echo $TEST_TMPDIR"));
  INFO("pwd output: " << exec("pwd"));
  INFO("ls output: " << exec("ls"));
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
