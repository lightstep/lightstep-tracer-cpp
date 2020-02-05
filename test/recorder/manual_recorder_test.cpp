#include "recorder/manual_recorder.h"

#include "3rd_party/catch2/catch.hpp"
#include "lightstep/tracer.h"
#include "test/recorder/in_memory_async_transporter.h"
#include "test/utility.h"
#include "tracer/counting_metrics_observer.h"
#include "tracer/tracer_impl.h"
using namespace lightstep;

TEST_CASE("ManualRecorder") {
  Logger logger{};
  auto metrics_observer = new CountingMetricsObserver{};
  LightStepTracerOptions options;
  size_t max_buffered_spans{5};
  options.max_buffered_spans =
      std::function<size_t()>{[&] { return max_buffered_spans; }};
  options.metrics_observer.reset(metrics_observer);
  std::shared_ptr<LightStepTracer> tracer;
  bool flush_on_full = true;
  auto on_span_buffer_full = [&] {
    if (flush_on_full) {
      tracer->Flush();
    }
  };
  auto in_memory_transporter =
      new InMemoryAsyncTransporter{on_span_buffer_full};
  auto recorder = new ManualRecorder{
      logger, std::move(options),
      std::unique_ptr<AsyncTransporter>{in_memory_transporter}};
  tracer = std::shared_ptr<LightStepTracer>{new TracerImpl{
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

  SECTION("Flush is called when the tracer's buffer is filled.") {
    for (size_t i = 0; i < max_buffered_spans + 1; ++i) {
      auto span = tracer->StartSpan("abc");
      CHECK(span);
      span->Finish();
    }
    in_memory_transporter->Succeed();
    CHECK(in_memory_transporter->reports().size() == 1);
  }

  SECTION("If we don't flush when the span buffer fills, we'll drop the span") {
    flush_on_full = false;
    for (size_t i = 0; i < max_buffered_spans + 1; ++i) {
      auto span = tracer->StartSpan("abc");
      CHECK(span);
      span->Finish();
    }
    REQUIRE(metrics_observer->num_spans_dropped == 1);
  }

  SECTION(
      "MetricsObserver::OnFlush gets called whenever the recorder is "
      "successfully flushed.") {
    auto span = tracer->StartSpan("abc");
    span->Finish();
    tracer->Flush();
    in_memory_transporter->Succeed();
    CHECK(metrics_observer->num_flushes == 1);
  }

  SECTION(
      "MetricsObserver::OnSpansSent gets called with the number of spans "
      "transported") {
    auto span1 = tracer->StartSpan("abc");
    span1->Finish();
    auto span2 = tracer->StartSpan("abc");
    span2->Finish();
    tracer->Flush();
    in_memory_transporter->Succeed();
    CHECK(metrics_observer->num_spans_sent == 2);
  }

  SECTION(
      "MetricsObserver::OnSpansDropped gets called when spans are dropped.") {
    logger.set_level(LogLevel::off);
    auto span1 = tracer->StartSpan("abc");
    span1->Finish();
    auto span2 = tracer->StartSpan("abc");
    span2->Finish();
    tracer->Flush();
    in_memory_transporter->Fail();
    CHECK(metrics_observer->num_spans_sent == 0);
    CHECK(metrics_observer->num_spans_dropped == 2);
  }
}
