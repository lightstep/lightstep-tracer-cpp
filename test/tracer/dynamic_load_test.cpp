#include "opentracing/dynamic_load.h"

#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc/grpc.h>

#include "test/tracer/in_memory_collector.h"

#include "3rd_party/catch2/catch.hpp"

using namespace lightstep;

const char* const server_address = "0.0.0.0:50051";

TEST_CASE("dynamic_load") {
  std::string error_message;

  auto handle_maybe = opentracing::DynamicallyLoadTracingLibrary(
      "test/tracer/lightstep_plugin.so", error_message);
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
    // Set up a test collector service.
    InMemoryCollector collector_service;
    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&collector_service);
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());

    // Create a tracer.
    const char* config = R"(
    {
      "component_name" : "dynamic_load_test",
      "access_token": "abc123",
      "collector_host": "0.0.0.0",
      "collector_port": 50051,
      "collector_plaintext": true
    })";
    auto tracer_maybe = tracer_factory.MakeTracer(config, error_message);
    REQUIRE(error_message.empty());
    REQUIRE(tracer_maybe);
    auto& tracer = *tracer_maybe;

    auto span = tracer->StartSpan("abc");
    span->Finish();
    tracer->Close();
    auto spans = collector_service.spans();
    CHECK(spans.size() == 1);
  }
}
