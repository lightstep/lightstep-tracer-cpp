#include "recorder/stream_recorder/stream_recorder.h"

#include <sys/types.h>
#include <unistd.h>

#include "test/mock_satellite/mock_satellite_handle.h"
#include "test/ports.h"
#include "test/utility.h"

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

TEST_CASE("StreamRecorder can be forked") {
  LightStepTracerOptions tracer_options;
  tracer_options.access_token = "abc123";
  tracer_options.collector_plaintext = true;
  tracer_options.satellite_endpoints = {
      {"localhost",
       static_cast<uint16_t>(PortAssignments::StreamRecorderForkTest)}};
  tracer_options.use_stream_recorder = true;
  auto tracer = MakeLightStepTracer(std::move(tracer_options));
  REQUIRE(tracer != nullptr);

  tracer->StartSpan("abc");
  if (::fork() == 0) {
    tracer->StartSpan("xyz");
    tracer->Close();
    std::exit(0);
  }
  std::unique_ptr<MockSatelliteHandle> mock_satellite{new MockSatelliteHandle{
      static_cast<uint16_t>(PortAssignments::StreamRecorderForkTest)}};
  tracer->Close();
  REQUIRE(
      IsEventuallyTrue([&] { return mock_satellite->spans().size() == 2; }));
}
