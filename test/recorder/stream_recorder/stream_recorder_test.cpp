#include "recorder/stream_recorder/stream_recorder.h"
#include "test/mock_satellite/mock_satellite_handle.h"
#include "test/ports.h"
#include "test/utility.h"
#include "tracer/lightstep_tracer_impl.h"

#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

TEST_CASE("StreamRecorder") {
  MockSatelliteHandle mock_satellite{
      static_cast<uint16_t>(PortAssignments::StreamRecorderTest)};

  // Testing stub. More will be added when StreamRecorder is filled out.
  auto logger = std::make_shared<Logger>();
  LightStepTracerOptions tracer_options;
  tracer_options.satellite_endpoints = {
      {"localhost",
       static_cast<uint16_t>(PortAssignments::StreamRecorderTest)}};
  StreamRecorderOptions recorder_options;
  recorder_options.flushing_period =
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::milliseconds{100});
  auto stream_recorder = new StreamRecorder{*logger, std::move(tracer_options),
                                            std::move(recorder_options)};
  std::unique_ptr<Recorder> recorder{stream_recorder};
  PropagationOptions propagation_options;
  auto tracer = std::make_shared<LightStepTracerImpl>(
      logger, propagation_options, std::move(recorder));
  auto span = tracer->StartSpan("abc");
  span->Finish();
  REQUIRE(!stream_recorder->empty());
  auto is_recorder_empty = [&stream_recorder] {
    return stream_recorder->empty();
  };
  REQUIRE(IsEventuallyTrue(is_recorder_empty));
}
