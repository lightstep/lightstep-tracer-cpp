#include "recorder/stream_recorder/stream_recorder.h"
#include "3rd_party/catch2/catch.hpp"
#include "tracer/lightstep_tracer_impl.h"

using namespace lightstep;

TEST_CASE("StreamRecorder") {
  // Testing stub. More will be added when StreamRecorder is filled out.
  auto logger = std::make_shared<Logger>();
  LightStepTracerOptions tracer_options;
  PropagationOptions propagation_options;
  std::unique_ptr<Recorder> recorder{
      new StreamRecorder{*logger, std::move(tracer_options)}};
  auto tracer = std::make_shared<LightStepTracerImpl>(
      logger, propagation_options, std::move(recorder));
  auto span = tracer->StartSpan("abc");
  span->Finish();
}
