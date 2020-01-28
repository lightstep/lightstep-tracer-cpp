#include "recorder/manual_recorder.h"

#include "test/recorder/in_memory_async_transporter.h"
#include "tracer/counting_metrics_observer.h"
#include "tracer/tracer_impl.h"
#include "test/utility.h"
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

  SECTION("Buffered spans get transported after Flush is manually called.") {
    auto span = tracer->StartSpan("abc");
    CHECK(span);
    span->Finish();
    CHECK(in_memory_transporter->reports().empty());
    CHECK(tracer->Flush());
    in_memory_transporter->Succeed();
    CHECK(in_memory_transporter->reports().size() == 1);
  }

  SECTION(
      "If the tranporter fails, it's spans are reported as dropped in the "
      "following report.") {
    logger.set_level(LogLevel::off);
    auto span1 = tracer->StartSpan("abc");
    CHECK(span1);
    span1->Finish();
    CHECK(tracer->Flush());
    in_memory_transporter->Fail();

    auto span2 = tracer->StartSpan("xyz");
    CHECK(span2);
    span2->Finish();
    CHECK(tracer->Flush());
    in_memory_transporter->Succeed();
    CHECK(LookupSpansDropped(in_memory_transporter->reports().at(0)) == 1);
  }

#if 0
  SECTION("Flush is called when the tracer's buffer is filled.") {
    for (size_t i = 0; i < max_buffered_spans; ++i) {
      auto span = tracer->StartSpan("abc");
      CHECK(span);
      span->Finish();
    }
    in_memory_transporter->Write();
    CHECK(in_memory_transporter->reports().size() == 1);
  }
#endif
}
