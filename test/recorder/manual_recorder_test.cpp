#include "recorder/manual_recorder.h"

#include "test/recorder/in_memory_async_transporter.h"
#include "tracer/counting_metrics_observer.h"
#include "tracer/tracer_impl.h"
#include "lightstep/tracer.h"
#include "3rd_party/catch2/catch.hpp"
using namespace lightstep;

TEST_CASE("ManualRecorder") {
  Logger logger{};
  auto metrics_observer = new CountingMetricsObserver{};
  LightStepTracerOptions options;
  size_t max_buffered_spans{5};
  options.max_buffered_spans =
      std::function<size_t()>{[&] { return max_buffered_spans; }};
  options.metrics_observer.reset(metrics_observer);
  auto in_memory_transporter = new InMemoryAsyncTransporter{};
  auto recorder = new ManualRecorder{
      logger, std::move(options),
      std::unique_ptr<AsyncTransporter>{in_memory_transporter}};
  auto tracer = std::shared_ptr<LightStepTracer>{new TracerImpl{
      PropagationOptions{}, std::unique_ptr<Recorder>{recorder}}};
  REQUIRE(tracer);
}
